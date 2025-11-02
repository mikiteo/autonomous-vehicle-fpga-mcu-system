library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity spi_master_top is
    port (
        clk         : in  std_logic;
        rst         : in  std_logic;
        sck         : out std_logic;
        mosi        : out std_logic;
        cs          : out std_logic;
        ce          : out std_logic;
        we          : out std_logic;
        addr        : out std_logic_vector(9 downto 0);
        data_ready  : in  std_logic;
        data_in     : in  std_logic_vector(15 downto 0)
    );
end spi_master_top;

architecture rtl of spi_master_top is

    type state_type is (idle, start_read, waiting_data, first_byte, second_byte, ce_wait);

    signal state     : state_type := idle;
    signal addr_cnt  : std_logic_vector(9 downto 0) := (others => '0');
    signal bit_cnt   : integer := 0;
    signal cnt       : integer := 0;
    signal index     : integer := 0;
    signal i         : integer := 0;
    signal cnt_wait  : integer := 0;
    signal cs_cnt    : integer := 0;
    signal sck_cnt   : unsigned (5 downto 0) := (others => '0');
    signal sck_ff    : std_logic := '0';
    signal shift_reg_16 : std_logic_vector(15 downto 0) := (others => '0');
    signal shift_reg : std_logic_vector(7 downto 0) := (others => '0');
    signal sck_reg   : std_logic := '0';
    signal high_byte : std_logic_vector(7 downto 0);
    signal low_byte  : std_logic_vector(7 downto 0);
    type frame_array is array(0 to 2) of std_logic_vector(15 downto 0);
    signal cmd_buffer : frame_array := (
        0 => x"55AA",
        1 => x"AA00",
        2 => x"8113"
    );

begin

    we <= '0';
    sck <= sck_reg;
    
    process(clk)
    begin
        if rst = '1' then
            state     <= idle;
            addr_cnt  <= (others => '0');
            bit_cnt   <= 0;
            index     <= 0;
            shift_reg <= (others => '0');
            sck_reg   <= '0';
            sck_ff    <= '0';
            i         <= 0;
        elsif rising_edge(clk) then
            case state is
                when idle =>
                    cs <= '1';
                    mosi <= '0';
                    sck_reg <= '0';
                    addr <= (others => '0');
                    if data_ready = '1' then
                        ce <= '1';
                        state  <= start_read;
                    end if;

                when start_read =>
                    addr <= addr_cnt;
                    if index = 3 then
                        index <= 0;
                        addr_cnt <= (others => '0');
                        cs <= '1';
                        state <= idle;
                    else
                        state <= waiting_data;
                        cs <= '0'; 
                    end if;

                when waiting_data =>
                    i <= i + 1;
                    if i = 2 then
                        i <= 0;
                        high_byte <= data_in(15 downto 8);
                        low_byte  <= data_in(7 downto 0);
                        state <= first_byte;
                    end if;

                when first_byte =>
                    sck_cnt <= sck_cnt + 1;
                    sck_reg <= sck_cnt(5);
                    sck_ff <= sck_reg;

                    if bit_cnt = 8 and sck_ff ='0' then
                        mosi <= '0';
                        state <= second_byte;
                        cs <= '1';
                        sck_reg <= '0';
                        state <= ce_wait;
                    end if;

                    if sck_reg = '1' and sck_ff ='0' then
                        mosi <= high_byte(7);
                        high_byte <= high_byte(6 downto 0) & '0';
                        bit_cnt <= bit_cnt + 1;
                    end if;

                when ce_wait =>
                    if cnt_wait = 1000 then
                        cs <= '0';
                        cnt_wait <= 0;
                        state  <= second_byte;
                        if bit_cnt = 16 then
                            bit_cnt <= 0;
                            index <= index + 1;
                            cs <= '1';
                            addr_cnt <= std_logic_vector(unsigned(addr_cnt) + 1);
                            state <= start_read;
                        end if;
                    else 
                        cnt_wait <= cnt_wait + 1;
                    end if;

                when second_byte =>

                    sck_cnt <= sck_cnt + 1;
                    sck_reg <= sck_cnt(5);
                    sck_ff <= sck_reg;

                    if bit_cnt = 16 and sck_ff ='0' then
                        sck_reg <= '0';
                        mosi <= '0';
                        cs <= '1';
                        state <= ce_wait;
                    end if;

                    if sck_reg = '1' and sck_ff ='0' then
                        mosi <= low_byte(7);
                        low_byte <= low_byte(6 downto 0) & '0';
                        bit_cnt <= bit_cnt + 1;
                    end if;

                when others =>
                    state <= idle;

            end case;
        end if;
    end process;
end rtl;

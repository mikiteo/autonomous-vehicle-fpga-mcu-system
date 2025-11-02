library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.ram_pkg.all;

entity uart_packet_receiver is
    generic (
        RAM_DEPTH   : integer := 1024
    );
    port (
        clk         : in  std_logic;
        rst         : in  std_logic;
        rx_data     : in  std_logic_vector(7 downto 0);
        rx_ready    : in  std_logic;
        final_cmd   : out std_logic;
        we_ce       : out std_logic;
        addr        : out std_logic_vector(clogb2(RAM_DEPTH - 1) downto 0);
        dout        : out std_logic_vector(15 downto 0)
    );
end entity;

architecture rtl of uart_packet_receiver is

    signal packet           : std_logic_vector(63 downto 0) := (others => '0');
    signal addr_cnt         : unsigned(clogb2(RAM_DEPTH) - 1 downto 0) := (others => '0');
    signal data_corr_toggle : std_logic := '0';
    signal rx_ready_reg     : std_logic_vector(3 downto 0) := (others => '0');
    signal detected         : std_logic := '0';
    signal we_ce_reg        : std_logic;

begin

    process(clk)
    begin
        if rising_edge(clk) then
            if rst = '1' then
                packet <= (others => '0');
            elsif rx_ready = '1' then
                packet <= packet(55 downto 0) & rx_data;
            end if;
        end if;
    end process;

    process(clk)
    begin
        if rst = '1' then
            rx_ready_reg <= (others => '0');
        elsif rising_edge(clk) then
            rx_ready_reg <= rx_ready & rx_ready_reg(rx_ready_reg'left downto 1);
        end if;
    end process;

    process(packet)
    begin
        if packet(63 downto 56) = x"55" and
           packet(55 downto 48) = x"AA" and
           packet(7 downto 0)   = x"FA" then
            detected <= '1';
        else
            detected <= '0';
        end if;
    end process;

    we_ce_reg <= '1' when (rx_ready_reg(0) = '1' and detected = '1') else '0';

    dout <= packet(31 downto 16);

    process(clk)
    begin
        if rising_edge(clk) then
            if rst = '1' then
                addr_cnt <= (others => '0');
            elsif we_ce_reg = '1' then
                addr_cnt <= addr_cnt + 1;
            end if;
        end if;
    end process;
    
    we_ce <= we_ce_reg;
    addr <= std_logic_vector(addr_cnt);
    final_cmd <= '1' when (rx_ready_reg(0) = '1' and detected = '1') else '0';
    
end architecture;
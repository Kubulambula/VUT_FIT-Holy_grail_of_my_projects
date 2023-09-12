-- uart_rx.vhd: UART controller - receiving (RX) side
-- Author(s): Jakub Janšta (xjanst02)

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.numeric_std.all;



-- Entity declaration (DO NOT ALTER THIS PART!)
entity UART_RX is
    port(
        CLK      : in std_logic;
        RST      : in std_logic;
        DIN      : in std_logic;
        DOUT     : out std_logic_vector(7 downto 0);
        DOUT_VLD : out std_logic
    );
end entity;



-- Architecture implementation (INSERT YOUR IMPLEMENTATION HERE)
architecture behavioral of UART_RX is

    signal mid_bit_counter : std_logic_vector(4 downto 0) := "00000";
    signal next_bit_counter : std_logic_vector(3 downto 0) := "0000";
    signal read_bit_index_counter : std_logic_vector(2 downto 0) := "000";

    signal shared_state : std_logic_vector(2 downto 0) := "000";

begin

    -- Instance of RX FSM
    fsm: entity work.UART_RX_FSM
    port map (
        CLK => CLK,
        RST => RST,
        DIN => DIN,
        -- counters
        MID_BIT_COUNTER => mid_bit_counter,
        NEXT_BIT_COUNTER => next_bit_counter,
        READ_BIT_INDEX_COUNTER => read_bit_index_counter,
        -- shared_state used to communicate the state from uart_rx_fsm
        SHARED_STATE => shared_state
    );
    
    --DOUT <= (others => '0');
    --DOUT_VLD <= '0';

    process(CLK) begin
    
        if rising_edge(CLK) then
            
            case shared_state is 

                when "000" => -- WAIT_FOR_START_BIT
                    -- reset everything
                    DOUT_VLD <= '0';
                    mid_bit_counter <= "00000";
                    next_bit_counter <= "0000";
                    read_bit_index_counter <= "000";

                when "001" => -- WAIT_FOR_FIRST_MIDBIT
                    mid_bit_counter <= mid_bit_counter + 1;

                when "010" => -- READ_DATA
                    DOUT(to_integer(unsigned(read_bit_index_counter))) <= DIN;
                    next_bit_counter <= "0000"; -- Reset the next_bit_counter so that it does not shift over time
                    read_bit_index_counter <= read_bit_index_counter + 1;
                
                when "011" => -- WAIT_FOR_NEXT_BIT
                    next_bit_counter <= next_bit_counter + 1;

                when "100" => -- CHECK_END_BIT
                    -- All the logic is cointained inside the FSM, so there is nothing to do
                    null;

                when "101" => -- DATA_VALID
                    DOUT_VLD <= '1';
                
                when others => -- invalid state
                    null;

            end case;            
        
        end if;
    
    end process;

end architecture;

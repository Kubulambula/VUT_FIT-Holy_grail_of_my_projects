-- uart_rx_fsm.vhd: UART controller - finite state machine controlling RX side
-- Author(s): Jakub Janšta (xjanst02)

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.numeric_std.all;



entity UART_RX_FSM is
    port(
        CLK : in std_logic;
        RST : in std_logic;
        DIN : in std_logic;
        -- counters
        MID_BIT_COUNTER : in std_logic_vector(4 downto 0);
        NEXT_BIT_COUNTER : in std_logic_vector(3 downto 0);
        READ_BIT_INDEX_COUNTER : in std_logic_vector(2 downto 0);
        -- This vector is used to communicate the current state to the implementation itself
        -- With 3 bits we can store up to 8 states so it is more than enough for our 6 (state number coresponds to the value that would )
        SHARED_STATE : out std_logic_vector(2 downto 0)
    );
end entity;



architecture behavioral of UART_RX_FSM is

    type State_t is (WAIT_FOR_START_BIT, WAIT_FOR_FIRST_MIDBIT, READ_DATA, WAIT_FOR_NEXT_BIT, CHECK_END_BIT, DATA_VALID);
    signal state : State_t := WAIT_FOR_START_BIT;

begin

    -- A small hack so that it wouldn't be necessary to assign SHARED_STATE on each state change
    --SHARED_STATE <= std_logic_vector(to_unsigned(State_t'pos(state), 3));

    process(CLK, RST) begin

        if RST = '1' then
            state <= WAIT_FOR_START_BIT;
            SHARED_STATE <= "000";
        
        elsif (rising_edge(CLK)) then
            case state is
                
                when WAIT_FOR_START_BIT =>
                    if DIN = '0' then
                        state <= WAIT_FOR_FIRST_MIDBIT;
                        SHARED_STATE <= "001";
                    end if;
                
                when WAIT_FOR_FIRST_MIDBIT =>
                    if MID_BIT_COUNTER = "10111" then -- wait for 24 ticks which brings us to the first midbit (after start bit)
                        state <= READ_DATA;
                        SHARED_STATE <= "010";
                    end if;
                
                when READ_DATA =>
                    -- The next state is always WAIT_FOR_NEXT_BIT
                    state <= WAIT_FOR_NEXT_BIT;
                    SHARED_STATE <= "011";
                
                when WAIT_FOR_NEXT_BIT =>
                    if NEXT_BIT_COUNTER = "1110" then -- wait 15 ticks so that DIN has plenty time to change it's state (1 tick consumed by read state)
                        if READ_BIT_INDEX_COUNTER = "000" then
                            state <= CHECK_END_BIT;
                            SHARED_STATE <= "100";
                        else
                            state <= READ_DATA;
                            SHARED_STATE <= "010";
                        end if;
                    end if;
                
                when CHECK_END_BIT =>
                    if DIN = '1' then
                        -- Input was valid. State DATA_VALID will be held just for 1 cycle
                        state <= DATA_VALID;
                        SHARED_STATE <= "101";
                    else
                        -- Input was invalid. Don't enable DOUT_VLD and start waiting for a start bit
                        state <= WAIT_FOR_START_BIT;
                        SHARED_STATE <= "000";
                    end if;
                
                when DATA_VALID =>
                    -- The next state after 1 tick is always WAIT_FOR_START_BIT
                    state <= WAIT_FOR_START_BIT;
                    SHARED_STATE <= "000";
                    
            end case;
        
        end if;

    end process;

end architecture;

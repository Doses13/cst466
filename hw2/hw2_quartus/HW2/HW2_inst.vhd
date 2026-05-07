	component HW2 is
		port (
			clk_clk         : in  std_logic                    := 'X';             -- clk
			keys_export     : in  std_logic_vector(3 downto 0) := (others => 'X'); -- export
			leds_export     : out std_logic_vector(9 downto 0);                    -- export
			switches_export : in  std_logic_vector(9 downto 0) := (others => 'X')  -- export
		);
	end component HW2;

	u0 : component HW2
		port map (
			clk_clk         => CONNECTED_TO_clk_clk,         --      clk.clk
			keys_export     => CONNECTED_TO_keys_export,     --     keys.export
			leds_export     => CONNECTED_TO_leds_export,     --     leds.export
			switches_export => CONNECTED_TO_switches_export  -- switches.export
		);


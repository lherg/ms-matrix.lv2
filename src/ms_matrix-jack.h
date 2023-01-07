extern const LV2_Descriptor* lv2_descriptor(uint32_t index);
extern const LV2UI_Descriptor* lv2ui_descriptor(uint32_t index);

static const RtkLv2Description _plugin = {
	&lv2_descriptor, &lv2ui_descriptor
	, 0 // uint32_t dsp_descriptor_id
	, 0 // uint32_t gui_descriptor_id
	, "Mide Side matrix"
	, (const struct LV2Port[23])
	{
		{ "swap", CONTROL_IN	,0		,0		,1		,"Swap input"},	
		{ "mmute", CONTROL_IN	,0		,0		,1		,"Midle mute"},
		{ "smute", CONTROL_IN	,0		,0		,1		,"Side mute"},			
		{ "mphase", CONTROL_IN	,0		,0		,1		,"Midle hpf"},
		{ "sphase", CONTROL_IN	,0		,0		,1		,"Side hpf"},
		{ "mphase", CONTROL_IN	,0		,0		,1		,"Midle phase"},
		{ "sphase", CONTROL_IN	,0		,0		,1		,"Side phase"},
		{ "hmgain", CONTROL_IN	,-12.f	,-48.f	,12.f	,"High Midle Gain"},
		{ "hsgain", CONTROL_IN	,-48.f	,-48.f	,12.f	,"High Side Gain"},
		{ "fmgain", CONTROL_IN	,0.f	,-6.f	,6.f	,"Fine Midle Gain"},
		{ "fsgain", CONTROL_IN	,0.f	,-6.f	,6.f	,"Fine Side Gain"},
		{ "peakin0", CONTROL_OUT,-60.f	,-60.f	,20.f	,"Signal Peak In0"},
		{ "peakin1", CONTROL_OUT,-60.f	,-60.f	,20.f	,"Signal Peak In1"},
		{ "peakout0", CONTROL_OUT,-60.f	,-60.f	,20.f	,"Signal Peak Out0"},
		{ "peakout1", CONTROL_OUT,-60.f	,-60.f	,20.f	,"Signal Peak Out1"},
		{ "rmsin0", CONTROL_OUT,-60.f	,-60.f	,20.f	,"Signal Rms In0"},
		{ "rmsin1", CONTROL_OUT,-60.f	,-60.f	,20.f	,"Signal Rms In1"},
		{ "rmsout0", CONTROL_OUT,-60.f	,-60.f	,20.f	,"Signal Rms Out0"},
		{ "rmsout1", CONTROL_OUT,-60.f	,-60.f	,20.f	,"Signal Rms Out1"},		
		{ "in1", 	AUDIO_IN	,nan	,nan	,nan	,"Audio Input 0"},
		{ "out1", 	AUDIO_OUT	,nan	,nan	,nan	,"Audio Output 0"},
		{ "in2", 	AUDIO_IN	,nan	,nan	,nan	,"Audio Input 1"},
		{ "out2", 	AUDIO_OUT	,nan	,nan	,nan	,"Audio Output 1"},
	}
	, 23 // uint32_t nports_total
	, 2 // uint32_t nports_audio_in
	, 2 // uint32_t nports_audio_out
	, 0 // uint32_t nports_midi_in
	, 0 // uint32_t nports_midi_out
	, 0 // uint32_t nports_atom_in
	, 0 // uint32_t nports_atom_out
	, 19 // uint32_t nports_ctrl
	, 8 // uint32_t nports_ctrl_in
	, 11 // uint32_t nports_ctrl_out
	, 8192 // uint32_t min_atom_bufsiz
	, false // bool send_time_info
	, UINT32_MAX // uint32_t latency_ctrl_port
};


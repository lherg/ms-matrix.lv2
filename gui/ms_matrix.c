// https://github.com/lherg/tutos.ui.lv2 
// eg-amp-robtk.lv2
// gui/amp.c

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "lv2/lv2plug.in/ns/extensions/ui/ui.h"

#include "../src/ms_matrix.h"

#define RTK_URI MS_MATRIX_URI
#define RTK_GUI "ui"

typedef struct {
	LV2UI_Write_Function write;
	LV2UI_Controller     controller;
	LV2UI_Touch*         touch;

	PangoFontDescription* font[2];

	RobWidget* rw;
	RobWidget* amp_table;

	RobTkLbl*  midle_lbl;
	RobTkLbl*  side_lbl;	
	RobTkLbl*  hgain_lbl;
	RobTkLbl*  fgain_lbl;

	RobTkCBtn* swap_btn;

	RobTkCBtn* m_mute_btn;
	RobTkCBtn* s_mute_btn;

	RobTkCBtn* m_phase_btn;
	RobTkCBtn* s_phase_btn;

	RobTkCBtn* m_hpf_btn;
	RobTkCBtn* s_hpf_btn;

	RobTkDial* amp_hmid_gain;
	cairo_surface_t* amp_hmid_cr;

	RobTkDial* amp_hsid_gain;
	cairo_surface_t* amp_hsid_cr;

	RobTkDial* amp_fmid_gain;
	cairo_surface_t* amp_fmid_cr;

	RobTkDial* amp_fsid_gain;
	cairo_surface_t* amp_fsid_cr;

	RobWidget* meters_in;
	cairo_surface_t* meters_in_bg;
	cairo_surface_t* meters_in_fg;

	RobWidget* meters_out;
	cairo_surface_t* meters_out_bg;
	cairo_surface_t* meters_out_fg;

	float   _peakin0;
	float   _peakin1;	
	float 	max_peak_in;
		
	float   _rmsin0;
	float   _rmsin1;

	float   _peakout0;
	float   _peakout1;	
	float 	max_peak_out;
	
	float   _rmsout0;
	float   _rmsout1;

	bool disable_signals;
	bool is_activate;
} AmpUI;

/*****************************************************************************/
// UI

/* Remove an delete UI windows. */
static void
destroy_window(AmpUI* ui)
{
	cairo_surface_destroy (ui->meters_in_bg);
	cairo_surface_destroy (ui->meters_in_fg);	
	robwidget_destroy (ui->meters_in);
	cairo_surface_destroy (ui->meters_out_bg);
	cairo_surface_destroy (ui->meters_out_fg);	
	robwidget_destroy (ui->meters_out);

    robtk_lbl_destroy (ui->midle_lbl);
    robtk_lbl_destroy (ui->side_lbl);
    robtk_lbl_destroy (ui->hgain_lbl);
    robtk_lbl_destroy (ui->fgain_lbl);	

	robtk_dial_destroy (ui->amp_hmid_gain);
    robtk_dial_destroy (ui->amp_hsid_gain);
    robtk_dial_destroy (ui->amp_fmid_gain);
    robtk_dial_destroy (ui->amp_fsid_gain);	

	robtk_cbtn_destroy (ui->swap_btn);
	robtk_cbtn_destroy (ui->m_mute_btn);
	robtk_cbtn_destroy (ui->s_mute_btn);
	robtk_cbtn_destroy (ui->m_phase_btn);
	robtk_cbtn_destroy (ui->s_phase_btn);
	robtk_cbtn_destroy (ui->m_hpf_btn);
	robtk_cbtn_destroy (ui->s_hpf_btn);

    rob_table_destroy (ui->amp_table);
    rob_box_destroy (ui->rw);
}

static void
prepare_faceplates (AmpUI* ui)
{
	cairo_t* cr;
	float    xlp, ylp;

/* clang-format off */
#define INIT_DIAL_SF(VAR, W, H)                                             \
  VAR = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 2 * (W), 2 * (H)); \
  cr  = cairo_create (VAR);                                                 \
  cairo_scale (cr, 2.0, 2.0);                                               \
  CairoSetSouerceRGBA (c_trs);                                              \
  cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);                           \
  cairo_rectangle (cr, 0, 0, W, H);                                         \
  cairo_fill (cr);                                                          \
  cairo_set_operator (cr, CAIRO_OPERATOR_OVER);

#define DIALDOTS(V, XADD, YADD)                                \
  float ang = (-.75 * M_PI) + (1.5 * M_PI) * (V);              \
  xlp       = GED_CX + XADD + sinf (ang) * (GED_RADIUS + 3.0); \
  ylp       = GED_CY + YADD - cosf (ang) * (GED_RADIUS + 3.0); \
  cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);               \
  CairoSetSouerceRGBA (c_g80);                                 \
  cairo_set_line_width (cr, 2.5);                              \
  cairo_move_to (cr, rint (xlp) - .5, rint (ylp) - .5);        \
  cairo_close_path (cr);                                       \
  cairo_stroke (cr);

#define RESPLABLEL(V)                                             \
  {                                                               \
    DIALDOTS (V, 4.5, 15.5)                                       \
    xlp = rint (GED_CX + 4.5 + sinf (ang) * (GED_RADIUS + 9.5));  \
    ylp = rint (GED_CY + 15.5 - cosf (ang) * (GED_RADIUS + 9.5)); \
  }
  
	INIT_DIAL_SF (ui->amp_hmid_cr, GED_WIDTH + 8, GED_HEIGHT + 20);
	RESPLABLEL (0.00);
	write_text_full (cr, "-48", ui->font[0], xlp + 6, ylp, 0, 1, c_g80);
	RESPLABLEL (0.2);
	RESPLABLEL (0.4);	
	RESPLABLEL (0.6);
	RESPLABLEL (0.8);
	write_text_full (cr, "0", ui->font[0], xlp, ylp, 0, 2, c_g80);
	RESPLABLEL (1.0);
	write_text_full (cr, "+12", ui->font[0], xlp - 6, ylp, 0, 3, c_g80);
	cairo_destroy (cr);

	INIT_DIAL_SF (ui->amp_hsid_cr, GED_WIDTH + 8, GED_HEIGHT + 20);
	RESPLABLEL (0.00);
	write_text_full (cr, "-48", ui->font[0], xlp + 6, ylp, 0, 1, c_g80);
	RESPLABLEL (0.2);
	RESPLABLEL (0.4);	
	RESPLABLEL (0.6);
	RESPLABLEL (0.8);
	write_text_full (cr, "0", ui->font[0], xlp, ylp, 0, 2, c_g80);
	RESPLABLEL (1.0);
	write_text_full (cr, "+12", ui->font[0], xlp - 6, ylp, 0, 3, c_g80);
	cairo_destroy (cr);	

	INIT_DIAL_SF (ui->amp_fmid_cr, GED_WIDTH + 8, GED_HEIGHT + 20);
	RESPLABLEL (0.00);
	write_text_full (cr, "-6", ui->font[0], xlp + 6, ylp, 0, 1, c_g80);
	RESPLABLEL (0.25);
	RESPLABLEL (0.5);
	write_text_full (cr, "0", ui->font[0], xlp, ylp, 0, 2, c_g80);
	RESPLABLEL (.75);
	RESPLABLEL (1.0);
	write_text_full (cr, "+6", ui->font[0], xlp - 6, ylp, 0, 3, c_g80);
	cairo_destroy (cr);

	INIT_DIAL_SF (ui->amp_fsid_cr, GED_WIDTH + 8, GED_HEIGHT + 20);
	RESPLABLEL (0.00);
	write_text_full (cr, "-6", ui->font[0], xlp + 6, ylp, 0, 1, c_g80);
	RESPLABLEL (0.25);
	RESPLABLEL (0.5);
	write_text_full (cr, "0", ui->font[0], xlp, ylp, 0, 2, c_g80);
	RESPLABLEL (.75);
	RESPLABLEL (1.0);
	write_text_full (cr, "+6", ui->font[0], xlp - 6, ylp, 0, 3, c_g80);
	cairo_destroy (cr);	

#undef DIALDOTS
#undef INIT_DIAL_SF
#undef RESPLABLEL
}

static void
display_annotation (AmpUI* ui, RobTkDial* d, cairo_t* cr, const char* txt)
{
	int tw, th;
	cairo_save (cr);
	PangoLayout* pl = pango_cairo_create_layout (cr);
	pango_layout_set_font_description (pl, ui->font[0]);
	pango_layout_set_text (pl, txt, -1);
	pango_layout_get_pixel_size (pl, &tw, &th);
	cairo_translate (cr, d->w_width / 2, d->w_height - 2);
	cairo_translate (cr, -tw / 2.0, -th);
	cairo_set_source_rgba (cr, .0, .0, .0, .7);
	rounded_rectangle (cr, -1, -1, tw + 3, th + 1, 3);
	cairo_fill (cr);
	CairoSetSouerceRGBA (c_wht);
	pango_cairo_show_layout (cr, pl);
	g_object_unref (pl);
	cairo_restore (cr);
	cairo_new_path (cr);
}

static void
dial_annotation_db (RobTkDial* d, cairo_t* cr, void* data)
{
	AmpUI* ui = (AmpUI*)(data);
	char    txt[16];
	snprintf (txt, 16, "%5.1f dB", d->cur);
	display_annotation (ui, d, cr, txt);
}

/* Swap callback */
static bool
cb_swap_btn (RobWidget* w, void* handle)
{
	AmpUI* ui = (AmpUI*)handle;
	const float swap = robtk_cbtn_get_active (ui->swap_btn) ? 1.f : 0.f;
	if (ui->disable_signals) {
		return TRUE;
	}
	swap ? robtk_lbl_set_text(ui->midle_lbl, "MIDLE\nCh2") : robtk_lbl_set_text(ui->midle_lbl, "MIDLE\nCh1");
	swap ? robtk_lbl_set_text(ui->side_lbl, "SIDE\nCh1") : robtk_lbl_set_text(ui->side_lbl, "SIDE\nCh2");	
	ui->write (ui->controller,MTX_SWAP, sizeof (float), 0, (const void*)&swap);
	return TRUE;
}

/* Mute callback */
static bool
cb_m_mute_btn (RobWidget* w, void* handle)
{
	AmpUI* ui = (AmpUI*)handle;
	const float mute = robtk_cbtn_get_active (ui->m_mute_btn) ? 1.f : 0.f;
	if (ui->disable_signals) {
		return TRUE;
	}
	ui->write (ui->controller,MTX_MMUTE, sizeof (float), 0, (const void*)&mute);
	return TRUE;
}

static bool
cb_s_mute_btn (RobWidget* w, void* handle)
{
	AmpUI* ui = (AmpUI*)handle;
	const float mute = robtk_cbtn_get_active (ui->s_mute_btn) ? 1.f : 0.f;
	if (ui->disable_signals) {
		return TRUE;
	}
	ui->write (ui->controller,MTX_SMUTE, sizeof (float), 0, (const void*)&mute);
	return TRUE;
}

/* hpf callback */
static bool
cb_m_hpf_btn (RobWidget* w, void* handle)
{
	AmpUI* ui = (AmpUI*)handle;
	const float phase = robtk_cbtn_get_active (ui->m_hpf_btn) ? 1.f : 0.f;
	if (ui->disable_signals) {
		return TRUE;
	}
	ui->write (ui->controller,MTX_MHPF, sizeof (float), 0, (const void*)&phase);
	return TRUE;
}

static bool
cb_s_hpf_btn (RobWidget* w, void* handle)
{
	AmpUI* ui = (AmpUI*)handle;
	const float phase = robtk_cbtn_get_active (ui->s_hpf_btn) ? 1.f : 0.f;
	if (ui->disable_signals) {
		return TRUE;
	}
	ui->write (ui->controller,MTX_SHPF, sizeof (float), 0, (const void*)&phase);
	return TRUE;
}

/* Phase callback */
static bool
cb_m_phase_btn (RobWidget* w, void* handle)
{
	AmpUI* ui = (AmpUI*)handle;
	const float phase = robtk_cbtn_get_active (ui->m_phase_btn) ? 1.f : 0.f;
	if (ui->disable_signals) {
		return TRUE;
	}
	ui->write (ui->controller,MTX_MPHASE, sizeof (float), 0, (const void*)&phase);
	return TRUE;
}

static bool
cb_s_phase_btn (RobWidget* w, void* handle)
{
	AmpUI* ui = (AmpUI*)handle;
	const float phase = robtk_cbtn_get_active (ui->s_phase_btn) ? 1.f : 0.f;
	if (ui->disable_signals) {
		return TRUE;
	}
	ui->write (ui->controller,MTX_SPHASE, sizeof (float), 0, (const void*)&phase);
	return TRUE;
}

/* Gain callback */
static bool
cb_amp_hmid_gain(RobWidget* w, void* handle)
{
	AmpUI* ui = (AmpUI*)handle;
	float gain = robtk_dial_get_value (ui->amp_hmid_gain);
	if (ui->disable_signals) {
		return TRUE;
	}
	ui->write (ui->controller,MTX_HMGAIN, sizeof (float), 0, &gain);
	return TRUE;
}

static bool
cb_amp_hsid_gain(RobWidget* w, void* handle)
{
	AmpUI* ui = (AmpUI*)handle;
	float gain = robtk_dial_get_value (ui->amp_hsid_gain);
	if (ui->disable_signals) {
		return TRUE;
	}
	ui->write (ui->controller,MTX_HSGAIN, sizeof (float), 0, &gain);
	return TRUE;
}

static bool
cb_amp_fmid_gain(RobWidget* w, void* handle)
{
	AmpUI* ui = (AmpUI*)handle;
	float gain = robtk_dial_get_value (ui->amp_fmid_gain);
	if (ui->disable_signals) {
		return TRUE;
	}
	ui->write (ui->controller,MTX_FMGAIN, sizeof (float), 0, &gain);
	return TRUE;
}

static bool
cb_amp_fsid_gain(RobWidget* w, void* handle)
{
	AmpUI* ui = (AmpUI*)handle;
	float gain = robtk_dial_get_value (ui->amp_fsid_gain);
	if (ui->disable_signals) {
		return TRUE;
	}
	ui->write (ui->controller,MTX_FSGAIN, sizeof (float), 0, &gain);
	return TRUE;
}

#define M_HEADER_H 40
#define M_MG_TOP 6
#define M_GRAPH_H 324
#define M_MG_BOTTOM 10
#define M_GRAD_H 16
#define M_MG_SIDE 16
#define M_GRAPH_W 14
#define M_MG_MIDLE 6
#define M_PEAK_H 16
#define M_PEAK_PLOT_H 5
#define M_RMS_PLOT_H 3
#define M_H (M_HEADER_H + M_MG_TOP + M_GRAPH_H + M_MG_BOTTOM)
#define M_W_MONO (2*M_MG_SIDE+ M_GRAPH_W)
#define M_W_STEREO (2*M_MG_SIDE+ 2*M_GRAPH_W + M_MG_MIDLE)

static void
meters_in_size_request (RobWidget* rw, int* w, int* h)
{
	AmpUI* ui = (AmpUI*)GET_HANDLE (rw);
	*w = (M_W_STEREO)* ui->rw->widget_scale;
	*h = (M_H)* ui->rw->widget_scale;
}

static void
meters_out_size_request (RobWidget* rw, int* w, int* h)
{
	AmpUI* ui = (AmpUI*)GET_HANDLE (rw);
	*w = (M_W_STEREO)* ui->rw->widget_scale;
	*h = (M_H)* ui->rw->widget_scale;
}

static void
meters_in_size_allocate (RobWidget* rw, int w, int h)
{
	AmpUI* ui = (AmpUI*)GET_HANDLE (rw);
	if (ui->meters_in_bg) {
		cairo_surface_destroy (ui->meters_in_bg);
	}
	ui->meters_in_bg = NULL;

	queue_draw (ui->meters_in);
}

static void
meters_out_size_allocate (RobWidget* rw, int w, int h)
{
	AmpUI* ui = (AmpUI*)GET_HANDLE (rw);
	if (ui->meters_out_bg) {
		cairo_surface_destroy (ui->meters_out_bg);
	}
	ui->meters_out_bg = NULL;
	queue_draw (ui->meters_out);
}

static RobWidget* meters_in_mousedown (RobWidget* rw, RobTkBtnEvent *event) {
	AmpUI* ui = (AmpUI*)GET_HANDLE (rw);
	ui->max_peak_in = -60.0f;
	return NULL;
}

static RobWidget* meters_out_mousedown (RobWidget* rw, RobTkBtnEvent *event) {
	AmpUI* ui = (AmpUI*)GET_HANDLE (rw);
	ui->max_peak_out = -60.0f;
	return NULL;
}

static bool
meters_in_expose_event (RobWidget* rw, cairo_t* cr, cairo_rectangle_t* ev)
{
	AmpUI* ui = (AmpUI*)GET_HANDLE (rw);
	cairo_set_operator (cr, CAIRO_OPERATOR_OVER);

	float c[4];
	get_color_from_theme (1, c);
	cairo_set_source_rgb (cr, c[0], c[1], c[2]);

	// meters_in bg
	cairo_rectangle (cr, ev->x, ev->y, ev->width, ev->height);
	cairo_fill_preserve (cr);	
	cairo_clip (cr);
	
	// meters_in Graph1
	CairoSetSouerceRGBA (c_blk);
	cairo_rectangle (cr, M_MG_SIDE, M_HEADER_H + M_MG_TOP, M_GRAPH_W, M_GRAPH_H);
	cairo_rectangle (cr, M_MG_SIDE + M_MG_MIDLE + M_GRAPH_W , M_HEADER_H + M_MG_TOP ,M_GRAPH_W , M_GRAPH_H);
	cairo_fill (cr);

	// meters_in Grad 1
	CairoSetSouerceRGBA (c_wht);
	for (uint32_t d = 1; d <= 21; ++d) {
		const float y = M_GRAPH_H - (d-1) * 16.f + M_HEADER_H + M_MG_TOP;
		
		cairo_set_line_width (cr, 1.0);
		CairoSetSouerceRGBA (c_blk);
		cairo_move_to (cr, 10, y-3);
		cairo_line_to (cr, M_MG_SIDE, y-3);
		cairo_stroke (cr);

		cairo_set_line_width (cr, 1.0);
		CairoSetSouerceRGBA (c_blk);
		cairo_move_to (cr, M_MG_SIDE+2*M_GRAPH_W + M_MG_MIDLE, y-3);
		cairo_line_to (cr, M_MG_SIDE+2*M_GRAPH_W + M_MG_MIDLE+6, y-3);
		cairo_stroke (cr);

		char txt[16];
		snprintf (txt, 16, "%+2d", -64 + d * 4);
		write_text_full (cr, txt, ui->font[0], 6, y-3, 0, 5, c_blk);
		write_text_full (cr, txt, ui->font[0], M_MG_SIDE+2*M_GRAPH_W + M_MG_MIDLE + 6, y-3, 0, 5, c_blk);	
	}	
	cairo_fill (cr);

	// Peak
	char peak_lbl[16];
	snprintf (peak_lbl, 16, "PEAK DbFS");
	write_text_full (cr, peak_lbl, ui->font[1], M_MG_SIDE -7, 5, 0, M_GRAPH_W, c_blk);
	cairo_fill (cr);

	CairoSetSouerceRGBA (c_wht);
	if (ui->max_peak_in >= -1.0f) CairoSetSouerceRGBA (c_red);
	cairo_rectangle (cr, M_MG_SIDE, M_HEADER_H - M_PEAK_H -3, 2*M_GRAPH_W + M_MG_MIDLE, M_PEAK_H);
	cairo_fill (cr);

	char peak_txt[16];
	snprintf (peak_txt, 16, "%+2.1f", ui->max_peak_in);
	write_text_full (cr, peak_txt, ui->font[1], M_MG_SIDE + 2, M_HEADER_H - M_PEAK_H -1, 0, M_GRAPH_W, c_blk);
	cairo_fill (cr);

	float peakin0=-80.0f;
	float peakin1=-80.0f;	
	peakin0 = (ui->_peakin0 >= 1e-10f) ? 0 : ui->_peakin0;
	peakin1 = (ui->_peakin1 >= 1e-10f) ? 0 : ui->_peakin1;	
	cairo_set_source_rgb (cr, 0, 0, 1);
	cairo_rectangle (cr, M_MG_SIDE, M_HEADER_H + M_MG_TOP - 4*round(peakin0), M_GRAPH_W, 5);
	cairo_rectangle (cr, M_MG_SIDE + M_GRAPH_W + M_MG_MIDLE, M_HEADER_H + M_MG_TOP - 4*ceilf(peakin1), M_GRAPH_W, 5);	
	cairo_fill (cr);

	// Rms
	float rmsin0=-80.0f;
	float rmsin1=-80.0f;	
	rmsin0 = (ui->_peakin0 >= 1e-10f) ? 0 : ui->_rmsin0;
	rmsin1 = (ui->_peakin1 >= 1e-10f) ? 0 : ui->_rmsin1;		
	CairoSetSouerceRGBA (c_grn);
	uint32_t nbplots_in0 = (rmsin0 > -80.0 ? floor(81 + rmsin0) : 1) ;
	for (uint32_t n = 1; n <= nbplots_in0; ++n) {
		CairoSetSouerceRGBA (c_grn);
		if (n >= 60) CairoSetSouerceRGBA (c_nyl);
		if (n >= 64) CairoSetSouerceRGBA (c_red);
		cairo_rectangle (cr, M_MG_SIDE, M_HEADER_H + M_MG_TOP - 4 * (n - 81), M_GRAPH_W, 3);
		cairo_fill (cr);
	}
	uint32_t nbplots_in1 = (rmsin1 > -80.0 ? floor(81 + rmsin1) : 1) ;
	for (uint32_t n = 1; n <= nbplots_in1; ++n) {
		CairoSetSouerceRGBA (c_grn);
		if (n >= 60) CairoSetSouerceRGBA (c_nyl);
		if (n >= 64) CairoSetSouerceRGBA (c_red);
		cairo_rectangle (cr, M_MG_SIDE + M_GRAPH_W + M_MG_MIDLE, M_HEADER_H + M_MG_TOP - 4 * (n - 81), M_GRAPH_W, 3);
		cairo_fill (cr);
	}

	cairo_fill (cr);

	ui->is_activate = true;
	return TRUE;
}

static bool
meters_out_expose_event (RobWidget* rw, cairo_t* cr, cairo_rectangle_t* ev)
{
	AmpUI* ui = (AmpUI*)GET_HANDLE (rw);
	cairo_set_operator (cr, CAIRO_OPERATOR_OVER);

	float c[4];
	get_color_from_theme (1, c);
	cairo_set_source_rgb (cr, c[0], c[1], c[2]);

	// meters_out bg
	cairo_rectangle (cr, ev->x, ev->y, ev->width, ev->height);
	cairo_fill_preserve (cr);	
	cairo_clip (cr);

	// meters_out Graph1
	CairoSetSouerceRGBA (c_blk);
	cairo_rectangle (cr, M_MG_SIDE, M_HEADER_H + M_MG_TOP, M_GRAPH_W, M_GRAPH_H);
	cairo_rectangle (cr, M_MG_SIDE + M_MG_MIDLE + M_GRAPH_W , M_HEADER_H + M_MG_TOP ,M_GRAPH_W , M_GRAPH_H);
	cairo_fill (cr);

	// meters_out Grad 1
	CairoSetSouerceRGBA (c_wht);
	for (uint32_t d = 1; d <= 21; ++d) {
		const float y = M_GRAPH_H - (d-1) * 16.f + M_HEADER_H + M_MG_TOP;
		
		cairo_set_line_width (cr, 1.0);
		CairoSetSouerceRGBA (c_blk);
		cairo_move_to (cr, 10, y-3);
		cairo_line_to (cr, M_MG_SIDE, y-3);
		cairo_stroke (cr);

		cairo_set_line_width (cr, 1.0);
		CairoSetSouerceRGBA (c_blk);
		cairo_move_to (cr, M_MG_SIDE+2*M_GRAPH_W + M_MG_MIDLE, y-3);
		cairo_line_to (cr, M_MG_SIDE+2*M_GRAPH_W + M_MG_MIDLE+6, y-3);
		cairo_stroke (cr);

		char txt[16];
		snprintf (txt, 16, "%+2d", -64 + d * 4);
		write_text_full (cr, txt, ui->font[0], 6, y-3, 0, 5, c_blk);
		write_text_full (cr, txt, ui->font[0], M_MG_SIDE+2*M_GRAPH_W + M_MG_MIDLE + 6, y-3, 0, 5, c_blk);	
	}	
	cairo_fill (cr);

	// Peak
	char peak_lbl[16];
	snprintf (peak_lbl, 16, "PEAK DbFS");
	write_text_full (cr, peak_lbl, ui->font[1], M_MG_SIDE -7, 5, 0, M_GRAPH_W, c_blk);
	cairo_fill (cr);

	CairoSetSouerceRGBA (c_wht);
	if (ui->max_peak_out >= -1.0f) CairoSetSouerceRGBA (c_red);
	cairo_rectangle (cr, M_MG_SIDE, M_HEADER_H - M_PEAK_H -3, 2*M_GRAPH_W + M_MG_MIDLE, M_PEAK_H);
	cairo_fill (cr);

	char peak_txt[16];
	snprintf (peak_txt, 16, "%+2.1f", ui->max_peak_out);
	write_text_full (cr, peak_txt, ui->font[1], M_MG_SIDE + 2, M_HEADER_H - M_PEAK_H -1, 0, M_GRAPH_W, c_blk);
	cairo_fill (cr);

	float peakout0=-80.0f;
	float peakout1=-80.0f;	
	peakout0 = (ui->_peakout0 >= 1e-10f) ? 0 : ui->_peakout0;
	peakout1 = (ui->_peakout1 >= 1e-10f) ? 0 : ui->_peakout1;	
	cairo_set_source_rgb (cr, 0, 0, 1);
	cairo_rectangle (cr, M_MG_SIDE, M_HEADER_H + M_MG_TOP - 4*round(peakout0), M_GRAPH_W, 5);
	cairo_rectangle (cr, M_MG_SIDE + M_GRAPH_W + M_MG_MIDLE, M_HEADER_H + M_MG_TOP - 4*ceilf(peakout1), M_GRAPH_W, 5);	
	cairo_fill (cr);

	// Rms
	float rmsout0=-80.0f;
	float rmsout1=-80.0f;	
	rmsout0 = (ui->_rmsout0 >= 1e-10f) ? 0 : ui->_rmsout0;
	rmsout1 = (ui->_rmsout1 >= 1e-10f) ? 0 : ui->_rmsout1;		
	CairoSetSouerceRGBA (c_grn);
	uint32_t nbplots_out0 = (rmsout0 > -80.0 ? floor(81 + rmsout0) : 1) ;
	for (uint32_t n = 1; n <= nbplots_out0; ++n) {
		CairoSetSouerceRGBA (c_grn);
		if (n >= 60) CairoSetSouerceRGBA (c_nyl);
		if (n >= 64) CairoSetSouerceRGBA (c_red);
		cairo_rectangle (cr, M_MG_SIDE, M_HEADER_H + M_MG_TOP - 4 * (n - 81), M_GRAPH_W, 3);
		cairo_fill (cr);
	}
	uint32_t nbplots_out1 = (rmsout1 > -80.0 ? floor(81 + rmsout1) : 1) ;
	for (uint32_t n = 1; n <= nbplots_out1; ++n) {
		CairoSetSouerceRGBA (c_grn);
		if (n >= 60) CairoSetSouerceRGBA (c_nyl);
		if (n >= 64) CairoSetSouerceRGBA (c_red);
		cairo_rectangle (cr, M_MG_SIDE + M_GRAPH_W + M_MG_MIDLE, M_HEADER_H + M_MG_TOP - 4 * (n - 81), M_GRAPH_W, 3);
		cairo_fill (cr);
	}
	
	//cairo_rectangle (cr, M_MG_SIDE, M_HEADER_H + M_MG_TOP - 4*ceilf(ui->_rmsout0), M_GRAPH_W, 3);
	//cairo_rectangle (cr, M_MG_SIDE + M_GRAPH_W + M_MG_MIDLE, M_HEADER_H + M_MG_TOP - 4*ceilf(ui->_rmsout1), M_GRAPH_W, 3);	

	ui->is_activate = true;
	return TRUE;
}

static RobWidget*
toplevel (AmpUI* ui, void* const top)
{
	/* toplevel */
	ui->rw = rob_vbox_new (FALSE, 2);
	robwidget_make_toplevel (ui->rw, top);
	robwidget_toplevel_enable_scaling (ui->rw);

	ui->font[0] = pango_font_description_from_string ("Mono 8px");
	ui->font[1] = pango_font_description_from_string ("Mono 10px");

	/* toplevel */
	ui->amp_table = rob_table_new (/*rows*/ 13, /*cols*/ 4, FALSE);

	/* meters_in display */
	ui->meters_in = robwidget_new (ui);
	robwidget_set_alignment (ui->meters_in, .5, .5);
	robwidget_set_expose_event (ui->meters_in, meters_in_expose_event);
	robwidget_set_size_request (ui->meters_in, meters_in_size_request);
	robwidget_set_size_allocate (ui->meters_in, meters_in_size_allocate);

	/* meters_out display */
	ui->meters_out = robwidget_new (ui);
	robwidget_set_alignment (ui->meters_out, .5, .5);
	robwidget_set_expose_event (ui->meters_out, meters_out_expose_event);
	robwidget_set_size_request (ui->meters_out, meters_out_size_request);
	robwidget_set_size_allocate (ui->meters_out, meters_out_size_allocate);

	/* label */
	ui->midle_lbl = robtk_lbl_new ("MIDLE\nCh1");
	ui->side_lbl = robtk_lbl_new ("SIDE\nCh2");
	ui->hgain_lbl = robtk_lbl_new ("High Gain");
	ui->fgain_lbl = robtk_lbl_new ("Fine Gain");

	prepare_faceplates (ui);

	/* swap button */
	ui->swap_btn= robtk_cbtn_new ("Swap inputs", GBT_LED_RIGHT, false);
	robtk_cbtn_set_callback (ui->swap_btn, cb_swap_btn, ui);

	/* mute button */
	ui->m_mute_btn= robtk_cbtn_new ("mute", GBT_LED_RIGHT, false);
	robtk_cbtn_set_callback (ui->m_mute_btn, cb_m_mute_btn, ui);
	ui->s_mute_btn= robtk_cbtn_new ("mute", GBT_LED_RIGHT, false);
	robtk_cbtn_set_callback (ui->s_mute_btn, cb_s_mute_btn, ui);
			
	/* hpf button */
	ui->m_hpf_btn= robtk_cbtn_new ("hpf", GBT_LED_RIGHT, false);
	robtk_cbtn_set_callback (ui->m_hpf_btn, cb_m_hpf_btn, ui);
	ui->s_hpf_btn= robtk_cbtn_new ("hpf", GBT_LED_RIGHT, false);
	robtk_cbtn_set_callback (ui->s_hpf_btn, cb_s_hpf_btn, ui);

	/* phase button */
	ui->m_phase_btn= robtk_cbtn_new ("Ø", GBT_LED_RIGHT, false);
	robtk_cbtn_set_callback (ui->m_phase_btn, cb_m_phase_btn, ui);
	ui->s_phase_btn= robtk_cbtn_new ("Ø", GBT_LED_RIGHT, false);
	robtk_cbtn_set_callback (ui->s_phase_btn, cb_s_phase_btn, ui);

	/* dial */
	ui->amp_hmid_gain = robtk_dial_new_with_size (
		-48.0f, 12.0f, 6.0f,
		GED_WIDTH + 8, GED_HEIGHT + 20, GED_CX + 4, GED_CY + 15, GED_RADIUS);
	ui->amp_hsid_gain = robtk_dial_new_with_size (
		-48.0f, 12.0f, 6.0f,
		GED_WIDTH + 8, GED_HEIGHT + 20, GED_CX + 4, GED_CY + 15, GED_RADIUS);	
	ui->amp_fmid_gain = robtk_dial_new_with_size (
		-6.0f, 6.0f, 0.1f,
		GED_WIDTH + 8, GED_HEIGHT + 20, GED_CX + 4, GED_CY + 15, GED_RADIUS);
	ui->amp_fsid_gain = robtk_dial_new_with_size (
		-6.0f, 6.0f, 0.1f,
		GED_WIDTH + 8, GED_HEIGHT + 20, GED_CX + 4, GED_CY + 15, GED_RADIUS);	

	robtk_dial_set_default (ui->amp_hmid_gain, -12.0f);
	robtk_dial_set_callback (ui->amp_hmid_gain, cb_amp_hmid_gain, ui);
	float detent = 0.f;
	robtk_dial_set_detents (ui->amp_hmid_gain, 6 , &detent);
	robtk_dial_enable_states (ui->amp_hmid_gain, 1);
	robtk_dial_set_state_color (ui->amp_hmid_gain, 1, 1.0, .0, .0, .3);
	robtk_dial_set_default_state (ui->amp_hmid_gain, 0);

	robtk_dial_set_default (ui->amp_hsid_gain, -48.0f);
	robtk_dial_set_callback (ui->amp_hsid_gain, cb_amp_hsid_gain, ui);
	robtk_dial_set_detents (ui->amp_hsid_gain, 6 , &detent);
	robtk_dial_enable_states (ui->amp_hsid_gain, 1);
	robtk_dial_set_state_color (ui->amp_hsid_gain, 1, 1.0, .0, .0, .3);
	robtk_dial_set_default_state (ui->amp_hsid_gain, 0);

	robtk_dial_set_default (ui->amp_fmid_gain, 0.0f);
	robtk_dial_set_callback (ui->amp_fmid_gain, cb_amp_fmid_gain, ui);
	robtk_dial_set_detents (ui->amp_fmid_gain, 6 , &detent);
	robtk_dial_enable_states (ui->amp_fmid_gain, 1);
	robtk_dial_set_state_color (ui->amp_fmid_gain, 1, 1.0, .0, .0, .3);
	robtk_dial_set_default_state (ui->amp_fmid_gain, 0);

	robtk_dial_set_default (ui->amp_fsid_gain, 0.0f);
	robtk_dial_set_callback (ui->amp_fsid_gain, cb_amp_fsid_gain, ui);
	robtk_dial_set_detents (ui->amp_fsid_gain, 6 , &detent);
	robtk_dial_enable_states (ui->amp_fsid_gain, 1);
	robtk_dial_set_state_color (ui->amp_fsid_gain, 1, 1.0, .0, .0, .3);
	robtk_dial_set_default_state (ui->amp_fsid_gain, 0);

	robtk_dial_annotation_callback (ui->amp_hmid_gain, dial_annotation_db, ui);	
	robtk_dial_annotation_callback (ui->amp_hsid_gain, dial_annotation_db, ui);	
	robtk_dial_annotation_callback (ui->amp_fmid_gain, dial_annotation_db, ui);	
	robtk_dial_annotation_callback (ui->amp_fsid_gain, dial_annotation_db, ui);		

	ui->amp_hmid_gain->displaymode = 5;
	ui->amp_hsid_gain->displaymode = 5;	
	ui->amp_fmid_gain->displaymode = 5;
	ui->amp_fsid_gain->displaymode = 5;	

	if (ui->touch) {
		robtk_dial_set_touch (ui->amp_hmid_gain, ui->touch->touch, ui->touch->handle,MTX_OUTPUT0);
		robtk_dial_set_touch (ui->amp_hsid_gain, ui->touch->touch, ui->touch->handle,MTX_OUTPUT1);		
		robtk_dial_set_touch (ui->amp_fmid_gain, ui->touch->touch, ui->touch->handle,MTX_OUTPUT0);
		robtk_dial_set_touch (ui->amp_fsid_gain, ui->touch->touch, ui->touch->handle,MTX_OUTPUT1);
	}

	robtk_dial_set_scaled_surface_scale (ui->amp_hmid_gain, ui->amp_hmid_cr, 2.0);
	robtk_dial_set_scaled_surface_scale (ui->amp_hsid_gain, ui->amp_hsid_cr, 2.0);	
	robtk_dial_set_scaled_surface_scale (ui->amp_fmid_gain, ui->amp_fmid_cr, 2.0);
	robtk_dial_set_scaled_surface_scale (ui->amp_fsid_gain, ui->amp_fsid_cr, 2.0);	

	//Table meters_in
	rob_table_attach (ui->amp_table, ui->meters_in, 0, 1, 0, 13, 2, 2, RTK_FILL, RTK_FILL);
	//Table meters_out
	rob_table_attach (ui->amp_table, ui->meters_out, 3, 4, 0, 13, 2, 2, RTK_FILL, RTK_FILL);

	//Table line1 swap button
	rob_table_attach (ui->amp_table, robtk_cbtn_widget (ui->swap_btn)     , 1, 3, 2, 3, 8, 2, RTK_EXANDF, RTK_SHRINK);
	//Table line2 MIDLE SIDE label
	rob_table_attach (ui->amp_table, robtk_lbl_widget (ui->midle_lbl)     , 1, 2, 3, 4, 2, 2, RTK_SHRINK, RTK_SHRINK);
	rob_table_attach (ui->amp_table, robtk_lbl_widget (ui->side_lbl)      , 2, 3, 3, 4, 2, 2, RTK_SHRINK, RTK_SHRINK);
	//Table line3 mute button
	rob_table_attach (ui->amp_table, robtk_cbtn_widget (ui->m_mute_btn)   , 1, 2, 4, 5, 8, 2, RTK_EXANDF, RTK_SHRINK);
	rob_table_attach (ui->amp_table, robtk_cbtn_widget (ui->s_mute_btn)   , 2, 3, 4, 5, 8, 2, RTK_EXANDF, RTK_SHRINK);	
	//Table line4 hpf button
	rob_table_attach (ui->amp_table, robtk_cbtn_widget (ui->m_hpf_btn)    , 1, 2, 5, 6, 8, 2, RTK_EXANDF, RTK_SHRINK);
	rob_table_attach (ui->amp_table, robtk_cbtn_widget (ui->s_hpf_btn)    , 2, 3, 5, 6, 8, 2, RTK_EXANDF, RTK_SHRINK);
	//Table line5 phase button
	rob_table_attach (ui->amp_table, robtk_cbtn_widget (ui->m_phase_btn)  , 1, 2, 6, 7, 8, 2, RTK_EXANDF, RTK_SHRINK);
	rob_table_attach (ui->amp_table, robtk_cbtn_widget (ui->s_phase_btn)  , 2, 3, 6, 7, 8, 2, RTK_EXANDF, RTK_SHRINK);
	//Table line6 High Gain Label
	rob_table_attach (ui->amp_table, robtk_lbl_widget (ui->hgain_lbl)     , 1, 3, 7, 8, 2, 2, RTK_SHRINK, RTK_SHRINK);
	//Table line7 High Hain Knob
	rob_table_attach (ui->amp_table, robtk_dial_widget (ui->amp_hmid_gain),	1, 2, 8, 9,	0, 0, RTK_SHRINK, RTK_SHRINK);
	rob_table_attach (ui->amp_table, robtk_dial_widget (ui->amp_hsid_gain),	2, 3, 8, 9, 0, 0, RTK_SHRINK, RTK_SHRINK);
	//Table line8 Fine Gain Label
	rob_table_attach (ui->amp_table, robtk_lbl_widget (ui->fgain_lbl)     , 1, 3, 9, 10, 2, 2, RTK_SHRINK, RTK_SHRINK);
	//Table line9 Fine Hain Knob
	rob_table_attach (ui->amp_table, robtk_dial_widget (ui->amp_fmid_gain),	1, 2, 10, 11, 0, 0, RTK_SHRINK, RTK_SHRINK);
	rob_table_attach (ui->amp_table, robtk_dial_widget (ui->amp_fsid_gain),	2, 3, 10, 11,	0, 0, RTK_SHRINK, RTK_SHRINK);	

	/* top-level packing */
	rob_hbox_child_pack (ui->rw, ui->amp_table, TRUE, TRUE);

	robwidget_set_mousedown(ui->meters_in, meters_in_mousedown);
	robwidget_set_mousedown(ui->meters_out, meters_out_mousedown);

	return ui->rw;
}

/*****************************************************************************/
// LV2

#define LVGL_RESIZEABLE
static void ui_disable(LV2UI_Handle handle) { }
static void ui_enable(LV2UI_Handle handle) { }

static LV2UI_Handle
instantiate (
    void* const               ui_toplevel,
    const LV2UI_Descriptor*   descriptor,
    const char*               plugin_uri,
    const char*               bundle_path,
    LV2UI_Write_Function      write_function,
    LV2UI_Controller          controller,
    RobWidget**               widget,
    const LV2_Feature* const* features)
{
	AmpUI* ui = (AmpUI*)calloc (1, sizeof (AmpUI));
	if (!ui) {
		return NULL;
	}

	for (int i = 0; features[i]; ++i) {
		if (!strcmp (features[i]->URI, LV2_UI__touch)) {
			ui->touch = (LV2UI_Touch*)features[i]->data;
		}
	}

	ui->write           = write_function;
	ui->controller      = controller;
	ui->disable_signals = true;
	ui->is_activate = false;
	ui->max_peak_in = -60.0f;
	ui->max_peak_out = -60.0f;
	ui->_peakin0 = -60.0f;
	ui->_peakin1 = -60.0f;

	*widget             = toplevel (ui, ui_toplevel);
	ui->disable_signals = false;

	return ui;
}

static enum LVGLResize
plugin_scale_mode (LV2UI_Handle handle)
{
	return LVGL_LAYOUT_TO_FIT;
}

static void
cleanup(LV2UI_Handle handle)
{
	AmpUI* ui = (AmpUI*)handle;
	destroy_window (ui);
	free (ui);
}

/* receive information from DSP */
static void
port_event(LV2UI_Handle handle,
            uint32_t     port_index,
            uint32_t     buffer_size,
            uint32_t     format,
            const void*  buffer)
{
	AmpUI* ui = (AmpUI*)handle;
	if (port_index ==MTX_SWAP) {
		const float _swap = *(float*)buffer;
		ui->disable_signals = true;
		robtk_cbtn_set_active (ui->swap_btn, _swap > 0);
		ui->disable_signals = false;
	}
	if (port_index ==MTX_MMUTE) {
		const float _mute = *(float*)buffer;
		ui->disable_signals = true;
		robtk_cbtn_set_active (ui->m_mute_btn, _mute > 0);
		ui->disable_signals = false;
	}
	if (port_index ==MTX_SMUTE) {
		const float _mute = *(float*)buffer;
		ui->disable_signals = true;
		robtk_cbtn_set_active (ui->s_mute_btn, _mute > 0);
		ui->disable_signals = false;
	}		
	if (port_index ==MTX_MHPF) {
		const float _hpf = *(float*)buffer;
		ui->disable_signals = true;
		robtk_cbtn_set_active (ui->m_hpf_btn, _hpf > 0);
		ui->disable_signals = false;
	}
	if (port_index ==MTX_SHPF) {
		const float _hpf = *(float*)buffer;
		ui->disable_signals = true;
		robtk_cbtn_set_active (ui->s_hpf_btn, _hpf > 0);
		ui->disable_signals = false;
	}	
	if (port_index ==MTX_MPHASE) {
		const float _phase = *(float*)buffer;
		ui->disable_signals = true;
		robtk_cbtn_set_active (ui->m_phase_btn, _phase > 0);
		ui->disable_signals = false;
	}
	if (port_index ==MTX_SPHASE) {
		const float _phase = *(float*)buffer;
		ui->disable_signals = true;
		robtk_cbtn_set_active (ui->s_phase_btn, _phase > 0);
		ui->disable_signals = false;
	}	
	if (port_index ==MTX_HMGAIN) {
		const float _gain = *(float*)buffer;
		ui->disable_signals = true;
		robtk_dial_set_value (ui->amp_hmid_gain, _gain);
		ui->disable_signals = false;
	}
	if (port_index ==MTX_HSGAIN) {
		const float _gain = *(float*)buffer;
		ui->disable_signals = true;
		robtk_dial_set_value (ui->amp_hsid_gain, _gain);
		ui->disable_signals = false;
	}		
	if (port_index ==MTX_FMGAIN) {
		const float _gain = *(float*)buffer;
		ui->disable_signals = true;
		robtk_dial_set_value (ui->amp_fmid_gain, _gain);
		ui->disable_signals = false;
	}
	if (port_index ==MTX_FSGAIN) {
		const float _gain = *(float*)buffer;
		ui->disable_signals = true;
		robtk_dial_set_value (ui->amp_fsid_gain, _gain);
		ui->disable_signals = false;
	}
	if (port_index ==MTX_PEAKIN0) {
		ui->_peakin0 = *(float*)buffer;	
		if ((ui->_peakin0 > ui->max_peak_in) && ui->is_activate) ui->max_peak_in = ui->_peakin0;
		queue_draw (ui->meters_in);		
	}
	if (port_index ==MTX_PEAKIN1) {
		ui->_peakin1 = *(float*)buffer;
		if ((ui->_peakin1 > ui->max_peak_in) && ui->is_activate) ui->max_peak_in = ui->_peakin1;
		queue_draw (ui->meters_in);
	}
	if (port_index ==MTX_PEAKOUT0) {
		ui->_peakout0 = *(float*)buffer;		
		if ((ui->_peakout0 > ui->max_peak_out) && ui->is_activate) ui->max_peak_out = ui->_peakout0;
		queue_draw (ui->meters_out);
	}
	if (port_index ==MTX_PEAKOUT1) {
		ui->_peakout1 = *(float*)buffer;
		if ((ui->_peakout0 > ui->max_peak_out) && ui->is_activate) ui->max_peak_out = ui->_peakout0;
		queue_draw (ui->meters_out);
	}
	if (port_index ==MTX_RMSIN0) {
		ui->_rmsin0 = *(float*)buffer;
		queue_draw (ui->meters_in);
	}
	if (port_index ==MTX_RMSIN1) {
		ui->_rmsin1 = *(float*)buffer;
		queue_draw (ui->meters_in);
	}
	if (port_index ==MTX_RMSOUT0) {
		ui->_rmsout0 = *(float*)buffer;
		queue_draw (ui->meters_out);
	}
	if (port_index ==MTX_RMSOUT1) {
		ui->_rmsout1 = *(float*)buffer;
		queue_draw (ui->meters_out);
	}		
}

static const void*
extension_data(const char* uri)
{
	return NULL;
}


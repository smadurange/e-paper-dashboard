#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <stdlib.h>
#include <time.h>

#include <esp_log.h>

#include "dht.h"
#include "gui.h"
#include "scrn.h"
#include "stock.h"
#include "sprites/font_40.h"
#include "sprites/font_30.h"
#include "sprites/layout.h"

static const char *TAG = "gui";

static inline void gui_set_char(struct sprite *sp, char c, int bold)
{
	switch (c)
	{
	case 'A':
		if (bold) {
			sp->bmp = char_Ab_bmp;
			sp->width = char_Ab_width;
		} else {
			sp->bmp = char_A_bmp;
			sp->width = char_A_width;
		}
		break;
	case 'B':
		if (bold) {
			sp->bmp = char_Bb_bmp;
			sp->width = char_Bb_width;
		} else {
			sp->bmp = char_B_bmp;
			sp->width = char_B_width;
		}
		break;
	case 'C':
		if (bold) {
			sp->bmp = char_Cb_bmp;
			sp->width = char_Cb_width;
		} else {
			sp->bmp = char_C_bmp;
			sp->width = char_C_width;
		}
		break;
	case 'D':
		if (bold) {
			sp->bmp = char_Db_bmp;
			sp->width = char_Db_width;
		} else {
			sp->bmp = char_D_bmp;
			sp->width = char_D_width;
		}
		break;
	case 'E':
		if (bold) {
			sp->bmp = char_Eb_bmp;
			sp->width = char_Eb_width;
		} else {
			sp->bmp = char_E_bmp;
			sp->width = char_E_width;
		}
		break;
	case 'F':
		if (bold) {
			sp->bmp = char_Fb_bmp;
			sp->width = char_Fb_width;
		} else {
			sp->bmp = char_F_bmp;
			sp->width = char_F_width;
		}
		break;
	case 'G':
		if (bold) {
			sp->bmp = char_Gb_bmp;
			sp->width = char_Gb_width;
		} else {
			sp->bmp = char_G_bmp;
			sp->width = char_G_width;
		}
		break;
	case 'H':
		if (bold) {
			sp->bmp = char_Hb_bmp;
			sp->width = char_Hb_width;
		} else {
			sp->bmp = char_H_bmp;
			sp->width = char_H_width;
		}
		break;
	case 'I':
		if (bold) {
			sp->bmp = char_Ib_bmp;
			sp->width = char_Ib_width;
		} else {
			sp->bmp = char_I_bmp;
			sp->width = char_I_width;
		}
		break;
	case 'J':
		if (bold) {
			sp->bmp = char_Jb_bmp;
			sp->width = char_Jb_width;
		} else {
			sp->bmp = char_J_bmp;
			sp->width = char_J_width;
		}
		break;
	case 'K':
		if (bold) {
			sp->bmp = char_Kb_bmp;
			sp->width = char_Kb_width;
		} else {
			sp->bmp = char_K_bmp;
			sp->width = char_K_width;
		}
		break;
	case 'L':
		if (bold) {
			sp->bmp = char_Lb_bmp;
			sp->width = char_Lb_width;
		} else {
			sp->bmp = char_L_bmp;
			sp->width = char_L_width;
		}
		break;
	case 'M':
		if (bold) {
			sp->bmp = char_Mb_bmp;
			sp->width = char_Mb_width;
		} else {
			sp->bmp = char_M_bmp;
			sp->width = char_M_width;
		}
		break;
	case 'N':
		if (bold) {
			sp->bmp = char_Nb_bmp;
			sp->width = char_Nb_width;
		} else {
			sp->bmp = char_N_bmp;
			sp->width = char_N_width;
		}
		break;
	case 'O':
		if (bold) {
			sp->bmp = char_Ob_bmp;
			sp->width = char_Ob_width;
		} else {
			sp->bmp = char_O_bmp;
			sp->width = char_O_width;
		}
		break;
	case 'P':
		if (bold) {
			sp->bmp = char_Pb_bmp;
			sp->width = char_Pb_width;
		} else {
			sp->bmp = char_P_bmp;
			sp->width = char_P_width;
		}
		break;
	case 'Q':
		if (bold) {
			sp->bmp = char_Qb_bmp;
			sp->width = char_Qb_width;
		} else {
			sp->bmp = char_Q_bmp;
			sp->width = char_Q_width;
		}
		break;
	case 'R':
		if (bold) {
			sp->bmp = char_Rb_bmp;
			sp->width = char_Rb_width;
		} else {
			sp->bmp = char_R_bmp;
			sp->width = char_R_width;
		}
		break;
	case 'S':
		if (bold) {
			sp->bmp = char_Sb_bmp;
			sp->width = char_Sb_width;
		} else {
			sp->bmp = char_S_bmp;
			sp->width = char_S_width;
		}
		break;
	case 'T':
		if (bold) {
			sp->bmp = char_Tb_bmp;
			sp->width = char_Tb_width;
		} else {
			sp->bmp = char_T_bmp;
			sp->width = char_T_width;
		}
		break;
	case 'U':
		if (bold) {
			sp->bmp = char_Ub_bmp;
			sp->width = char_Ub_width;
		} else {
			sp->bmp = char_U_bmp;
			sp->width = char_U_width;
		}
		break;
	case 'V':
		if (bold) {
			sp->bmp = char_Vb_bmp;
			sp->width = char_Vb_width;
		} else {
			sp->bmp = char_V_bmp;
			sp->width = char_V_width;
		}
		break;
	case 'W':
		if (bold) {
			sp->bmp = char_Wb_bmp;
			sp->width = char_Wb_width;
		} else {
			sp->bmp = char_W_bmp;
			sp->width = char_W_width;
		}
		break;
	case 'X':
		if (bold) {
			sp->bmp = char_Xb_bmp;
			sp->width = char_Xb_width;
		} else {
			sp->bmp = char_X_bmp;
			sp->width = char_X_width;
		}
		break;
	case 'Y':
		if (bold) {
			sp->bmp = char_Yb_bmp;
			sp->width = char_Yb_width;
		} else {
			sp->bmp = char_Y_bmp;
			sp->width = char_Y_width;
		}
		break;
	case 'Z':
		if (bold) {
			sp->bmp = char_Zb_bmp;
			sp->width = char_Zb_width;
		} else {
			sp->bmp = char_Z_bmp;
			sp->width = char_Z_width;
		}
		break;
	case 'a':
		if (bold) {
			sp->bmp = char_ab_bmp;
			sp->width = char_ab_width;
		} else {
			sp->bmp = char_a_bmp;
			sp->width = char_a_width;
		}
		break;
	case 'b':
		if (bold) {
			sp->bmp = char_bb_bmp;
			sp->width = char_bb_width;
		} else {
			sp->bmp = char_b_bmp;
			sp->width = char_b_width;
		}
		break;
	case 'c':
		if (bold) {
			sp->bmp = char_cb_bmp;
			sp->width = char_cb_width;
		} else {
			sp->bmp = char_c_bmp;
			sp->width = char_c_width;
		}
		break;
	case 'd':
		if (bold) {
			sp->bmp = char_db_bmp;
			sp->width = char_db_width;
		} else {
			sp->bmp = char_d_bmp;
			sp->width = char_d_width;
		}
		break;
	case 'e':
		if (bold) {
			sp->bmp = char_eb_bmp;
			sp->width = char_eb_width;
		} else {
			sp->bmp = char_e_bmp;
			sp->width = char_e_width;
		}
		break;
	case 'f':
		if (bold) {
			sp->bmp = char_fb_bmp;
			sp->width = char_fb_width;
		} else {
			sp->bmp = char_f_bmp;
			sp->width = char_f_width;
		}
		break;
	case 'g':
		if (bold) {
			sp->bmp = char_gb_bmp;
			sp->width = char_gb_width;
		} else {
			sp->bmp = char_g_bmp;
			sp->width = char_g_width;
		}
		break;
	case 'h':
		if (bold) {
			sp->bmp = char_hb_bmp;
			sp->width = char_hb_width;
		} else {
			sp->bmp = char_h_bmp;
			sp->width = char_h_width;
		}
		break;
	case 'i':
		if (bold) {
			sp->bmp = char_ib_bmp;
			sp->width = char_ib_width;
		} else {
			sp->bmp = char_i_bmp;
			sp->width = char_i_width;
		}
		break;
	case 'j':
		if (bold) {
			sp->bmp = char_jb_bmp;
			sp->width = char_jb_width;
		} else {
			sp->bmp = char_j_bmp;
			sp->width = char_j_width;
		}
		break;
	case 'k':
		if (bold) {
			sp->bmp = char_kb_bmp;
			sp->width = char_kb_width;
		} else {
			sp->bmp = char_k_bmp;
			sp->width = char_k_width;
		}
		break;
	case 'l':
		if (bold) {
			sp->bmp = char_lb_bmp;
			sp->width = char_lb_width;
		} else {
			sp->bmp = char_l_bmp;
			sp->width = char_l_width;
		}
		break;
	case 'm':
		if (bold) {
			sp->bmp = char_mb_bmp;
			sp->width = char_mb_width;
		} else {
			sp->bmp = char_m_bmp;
			sp->width = char_m_width;
		}
		break;
	case 'n':
		if (bold) {
			sp->bmp = char_nb_bmp;
			sp->width = char_nb_width;
		} else {
			sp->bmp = char_n_bmp;
			sp->width = char_n_width;
		}
		break;
	case 'o':
		if (bold) {
			sp->bmp = char_ob_bmp;
			sp->width = char_ob_width;
		} else {
			sp->bmp = char_o_bmp;
			sp->width = char_o_width;
		}
		break;
	case 'p':
		if (bold) {
			sp->bmp = char_pb_bmp;
			sp->width = char_pb_width;
		} else {
			sp->bmp = char_p_bmp;
			sp->width = char_p_width;
		}
		break;
	case 'q':
		if (bold) {
			sp->bmp = char_qb_bmp;
			sp->width = char_qb_width;
		} else {
			sp->bmp = char_q_bmp;
			sp->width = char_q_width;
		}
		break;
	case 'r':
		if (bold) {
			sp->bmp = char_rb_bmp;
			sp->width = char_rb_width;
		} else {
			sp->bmp = char_r_bmp;
			sp->width = char_r_width;
		}
		break;
	case 's':
		if (bold) {
			sp->bmp = char_sb_bmp;
			sp->width = char_sb_width;
		} else {
			sp->bmp = char_s_bmp;
			sp->width = char_s_width;
		}
		break;
	case 't':
		if (bold) {
			sp->bmp = char_tb_bmp;
			sp->width = char_tb_width;
		} else {
			sp->bmp = char_t_bmp;
			sp->width = char_t_width;
		}
		break;
	case 'u':
		if (bold) {
			sp->bmp = char_ub_bmp;
			sp->width = char_ub_width;
		} else {
			sp->bmp = char_u_bmp;
			sp->width = char_u_width;
		}
		break;
	case 'v':
		if (bold) {
			sp->bmp = char_vb_bmp;
			sp->width = char_vb_width;
		} else {
			sp->bmp = char_v_bmp;
			sp->width = char_v_width;
		}
		break;
	case 'w':
		if (bold) {
			sp->bmp = char_wb_bmp;
			sp->width = char_wb_width;
		} else {
			sp->bmp = char_w_bmp;
			sp->width = char_w_width;
		}
		break;
	case 'x':
		if (bold) {
			sp->bmp = char_xb_bmp;
			sp->width = char_xb_width;
		} else {
			sp->bmp = char_x_bmp;
			sp->width = char_x_width;
		}
		break;
	case 'y':
		if (bold) {
			sp->bmp = char_yb_bmp;
			sp->width = char_yb_width;
		} else {
			sp->bmp = char_y_bmp;
			sp->width = char_y_width;
		}
		break;
	case 'z':
		if (bold) {
			sp->bmp = char_zb_bmp;
			sp->width = char_zb_width;
		} else {
			sp->bmp = char_z_bmp;
			sp->width = char_z_width;
		}
		break;
	case '0':
		if (bold) {
			sp->bmp = char_0b_bmp;
			sp->width = char_0b_width;
		} else {
			sp->bmp = char_0_bmp;
			sp->width = char_0_width;
		}
		break;
	case '1':
		if (bold) {
			sp->bmp = char_1b_bmp;
			sp->width = char_1b_width;
		} else {
			sp->bmp = char_1_bmp;
			sp->width = char_1_width;
		}
		break;
	case '2':
		if (bold) {
			sp->bmp = char_2b_bmp;
			sp->width = char_2b_width;
		} else {
			sp->bmp = char_2_bmp;
			sp->width = char_2_width;
		}
		break;
	case '3':
		if (bold) {
			sp->bmp = char_3b_bmp;
			sp->width = char_3b_width;
		} else {
			sp->bmp = char_3_bmp;
			sp->width = char_3_width;
		}
		break;
	case '4':
		if (bold) {
			sp->bmp = char_4b_bmp;
			sp->width = char_4b_width;
		} else {
			sp->bmp = char_4_bmp;
			sp->width = char_4_width;
		}
		break;
	case '5':
		if (bold) {
			sp->bmp = char_5b_bmp;
			sp->width = char_5b_width;
		} else {
			sp->bmp = char_5_bmp;
			sp->width = char_5_width;
		}
		break;
	case '6':
		if (bold) {
			sp->bmp = char_6b_bmp;
			sp->width = char_6b_width;
		} else {
			sp->bmp = char_6_bmp;
			sp->width = char_6_width;
		}
		break;
	case '7':
		if (bold) {
			sp->bmp = char_7b_bmp;
			sp->width = char_7b_width;
		} else {
			sp->bmp = char_7_bmp;
			sp->width = char_7_width;
		}
		break;
	case '8':
		if (bold) {
			sp->bmp = char_8b_bmp;
			sp->width = char_8b_width;
		} else {
			sp->bmp = char_8_bmp;
			sp->width = char_8_width;
		}
		break;
	case '9':
		if (bold) {
			sp->bmp = char_9b_bmp;
			sp->width = char_9b_width;
		} else {
			sp->bmp = char_9_bmp;
			sp->width = char_9_width;
		}
		break;
	case '}':
		if (bold) {
			sp->bmp = char_rcurlbra_b_bmp;
			sp->width = char_rcurlbra_b_width;
		} else {
			sp->bmp = char_rcurlbra_bmp;
			sp->width = char_rcurlbra_width;
		}
		break;
	case '{':
		if (bold) {
			sp->bmp = char_lcurlbra_b_bmp;
			sp->width = char_lcurlbra_b_width;
		} else {
			sp->bmp = char_lcurlbra_bmp;
			sp->width = char_lcurlbra_width;
		}
		break;
	case '\\':
		if (bold) {
			sp->bmp = char_backslash_b_bmp;
			sp->width = char_backslash_b_width;
		} else {
			sp->bmp = char_backslash_bmp;
			sp->width = char_backslash_width;
		}
		break;
	case '`':
		if (bold) {
			sp->bmp = char_backtick_b_bmp;
			sp->width = char_backtick_b_width;
		} else {
			sp->bmp = char_backtick_bmp;
			sp->width = char_backtick_width;
		}
		break;
	case '\'':
		if (bold) {
			sp->bmp = char_quote_b_bmp;
			sp->width = char_quote_b_width;
		} else {
			sp->bmp = char_quote_bmp;
			sp->width = char_quote_width;
		}
		break;
	case '"':
		if (bold) {
			sp->bmp = char_dquote_b_bmp;
			sp->width = char_dquote_b_width;
		} else {
			sp->bmp = char_dquote_bmp;
			sp->width = char_dquote_width;
		}
		break;
	case '=':
		if (bold) {
			sp->bmp = char_eq_b_bmp;
			sp->width = char_eq_b_width;
		} else {
			sp->bmp = char_eq_bmp;
			sp->width = char_eq_width;
		}
		break;
	case '/':
		if (bold) {
			sp->bmp = char_fwdslash_b_bmp;
			sp->width = char_fwdslash_b_width;
		} else {
			sp->bmp = char_fwdslash_bmp;
			sp->width = char_fwdslash_width;
		}
		break;
	case '^':
		if (bold) {
			sp->bmp = char_caret_b_bmp;
			sp->width = char_caret_b_width;
		} else {
			sp->bmp = char_caret_bmp;
			sp->width = char_caret_width;
		}
		break;
	case '#':
		if (bold) {
			sp->bmp = char_hash_b_bmp;
			sp->width = char_hash_b_width;
		} else {
			sp->bmp = char_hash_bmp;
			sp->width = char_hash_width;
		}
		break;
	case '?':
		if (bold) {
			sp->bmp = char_qmark_b_bmp;
			sp->width = char_qmark_b_width;
		} else {
			sp->bmp = char_qmark_bmp;
			sp->width = char_qmark_width;
		}
		break;
	case '&':
		if (bold) {
			sp->bmp = char_amp_b_bmp;
			sp->width = char_amp_b_width;
		} else {
			sp->bmp = char_amp_bmp;
			sp->width = char_amp_width;
		}
		break;
	case '-':
		if (bold) {
			sp->bmp = char_minus_b_bmp;
			sp->width = char_minus_b_width;
		} else {
			sp->bmp = char_minus_bmp;
			sp->width = char_minus_width;
		}
		break;
	case '@':
		if (bold) {
			sp->bmp = char_at_b_bmp;
			sp->width = char_at_b_width;
		} else {
			sp->bmp = char_at_bmp;
			sp->width = char_at_width;
		}
		break;
	case '_':
		if (bold) {
			sp->bmp = char_uscore_b_bmp;
			sp->width = char_uscore_b_width;
		} else {
			sp->bmp = char_uscore_bmp;
			sp->width = char_uscore_width;
		}
		break;
	case ';':
		if (bold) {
			sp->bmp = char_semicolon_b_bmp;
			sp->width = char_semicolon_b_width;
		} else {
			sp->bmp = char_semicolon_bmp;
			sp->width = char_semicolon_width;
		}
		break;
	case '%':
		if (bold) {
			sp->bmp = char_percent_b_bmp;
			sp->width = char_percent_b_width;
		} else {
			sp->bmp = char_percent_bmp;
			sp->width = char_percent_width;
		}
		break;
	case '*':
		if (bold) {
			sp->bmp = char_asterisk_b_bmp;
			sp->width = char_asterisk_b_width;
		} else {
			sp->bmp = char_asterisk_bmp;
			sp->width = char_asterisk_width;
		}
		break;
	case '$':
		if (bold) {
			sp->bmp = char_dollar_b_bmp;
			sp->width = char_dollar_b_width;
		} else {
			sp->bmp = char_dollar_bmp;
			sp->width = char_dollar_width;
		}
		break;
	case '|':
		if (bold) {
			sp->bmp = char_vbar_b_bmp;
			sp->width = char_vbar_b_width;
		} else {
			sp->bmp = char_vbar_bmp;
			sp->width = char_vbar_width;
		}
		break;
	case ')':
		if (bold) {
			sp->bmp = char_rbra_b_bmp;
			sp->width = char_rbra_b_width;
		} else {
			sp->bmp = char_rbra_bmp;
			sp->width = char_rbra_width;
		}
		break;
	case '(':
		if (bold) {
			sp->bmp = char_lbra_b_bmp;
			sp->width = char_lbra_b_width;
		} else {
			sp->bmp = char_lbra_bmp;
			sp->width = char_lbra_width;
		}
		break;
	case ':':
		if (bold) {
			sp->bmp = char_colon_b_bmp;
			sp->width = char_colon_b_width;
		} else {
			sp->bmp = char_colon_bmp;
			sp->width = char_colon_width;
		}
		break;
	case '~':
		if (bold) {
			sp->bmp = char_tilde_b_bmp;
			sp->width = char_tilde_b_width;
		} else {
			sp->bmp = char_tilde_bmp;
			sp->width = char_tilde_width;
		}
		break;
	case '<':
		if (bold) {
			sp->bmp = char_lt_b_bmp;
			sp->width = char_lt_b_width;
		} else {
			sp->bmp = char_lt_bmp;
			sp->width = char_lt_width;
		}
		break;
	case '>':
		if (bold) {
			sp->bmp = char_gt_b_bmp;
			sp->width = char_gt_b_width;
		} else {
			sp->bmp = char_gt_bmp;
			sp->width = char_gt_width;
		}
		break;
	case ']':
		if (bold) {
			sp->bmp = char_rsqbra_b_bmp;
			sp->width = char_rsqbra_b_width;
		} else {
			sp->bmp = char_rsqbra_bmp;
			sp->width = char_rsqbra_width;
		}
		break;
	case '[':
		if (bold) {
			sp->bmp = char_lsqbra_b_bmp;
			sp->width = char_lsqbra_b_width;
		} else {
			sp->bmp = char_lsqbra_bmp;
			sp->width = char_lsqbra_width;
		}
		break;
	case ',':
		if (bold) {
			sp->bmp = char_comma_b_bmp;
			sp->width = char_comma_b_width;
		} else {
			sp->bmp = char_comma_bmp;
			sp->width = char_comma_width;
		}
		break;
	case '!':
		if (bold) {
			sp->bmp = char_exclm_b_bmp;
			sp->width = char_exclm_b_width;
		} else {
			sp->bmp = char_exclm_bmp;
			sp->width = char_exclm_width;
		}
		break;
	case '+':
		if (bold) {
			sp->bmp = char_plus_b_bmp;
			sp->width = char_plus_b_width;
		} else {
			sp->bmp = char_plus_bmp;
			sp->width = char_plus_width;
		}
		break;
	case '.':
		if (bold) {
			sp->bmp = char_period_b_bmp;
			sp->width = char_period_b_width;
		} else {
			sp->bmp = char_period_bmp;
			sp->width = char_period_width;
		}
		break;
	default:
		sp->bmp = NULL;
		sp->width = 0;
		ESP_LOGW(TAG, "unknown char: %c(%d)", c, c);
		break;
	}
}

int gui_draw_str(struct scrn *sc, const char *s, int x0, int y0, int x_max, int y_max, int bold)
{
	int i, x, y;
	struct sprite sp;

	int space_width = 10;

	sp.height = font_30_height;

	for (i = 0, x = x0, y = y0; s[i]; i++) {
		if (s[i] != ' ') {
			gui_set_char(&sp, s[i], bold);
			if (sp.width > 0) {
				if (sp.width + x < x_max) {
					sp.offset_x = x;
					sp.offset_y = y;
					x += sp.width;
				} else {
					sp.offset_x = x0;
					x = x0 + sp.width;
					y += sp.height;
					if (y > y_max)
						return y;
					sp.offset_y = y;
				}
				scrn_draw(sc, &sp);
			}
		} else {
			x += space_width;
		}
	}

	return y + sp.height;
} 

void gui_plot_stocks(struct scrn *sc, struct stock_item *data)
{
	int x_min = 18, x_max = 310;
	int y_min = 92, y_max = 356;

	gui_draw_str(sc, data->name, x_min, 43, x_max, y_max, 0);

	int col_n = x_max - x_min;
	int row_n = y_max - y_min;

	int line_width = 3;

	int price_min = data->price_min;
	int price_max = data->price_max;

	int x_step = col_n % data->prices_len >= data->prices_len / 2
	                 ?  col_n / data->prices_len + 1
	                 : col_n / data->prices_len;
	if (x_step == 0)
		x_step = 1;

	int dy = (price_max - price_min) / 100;

	int y_step = dy != 0 ? row_n / dy : 0;
	if (y_step == 0)
		y_step = 1;

	for (int i = 0, x = 0, y_prev = 0; i < data->prices_len && x < col_n; i++, x += x_step) {
		int y = row_n - ((data->prices[i] - price_min) / 100) * y_step;
		if (y < 0)
			y = line_width - 1;
		if (y >= row_n)
			y = row_n - 1;

		// vertical step
		if (x > 0) {
			if (y_prev < y) {
				for (int k = y_prev - line_width + 1; k <= y; k++) {
					for (int j = x - line_width / 2, c = 0; c < line_width; j++, c++) {
						int px = (k + y_min) * sc->width + (j + x_min);
						sc->fb[px / 8] |= (1ULL << (7 - px % 8));
					}
				}
			} else if (y_prev > y) {
				for (int k = y_prev; k > y - line_width; k--) {
					for (int j = x - line_width / 2, c = 0; c < line_width; j++, c++) {
						int px = (k + y_min) * sc->width + (j + x_min);
						sc->fb[px / 8] |= (1ULL << (7 - px % 8));
					}
				}
			}
		}

		// horizontal step
		for (int j = x; j < x + x_step; j++) {
			for (int k = y, c = 0; c < line_width; k--, c++) {
				int px = (k + y_min) * sc->width + (j + x_min);
				sc->fb[px / 8] |= (1ULL << (7 - px % 8));
			}
		}
		
		y_prev = y;

		// watchdog timer
		vTaskDelay(10 / portTICK_PERIOD_MS);	
	}

	// reference line
	int dash_len = 4;

	int price_ref = data->price_ref;
	if (price_ref < price_min)
		price_ref = price_min;
	else if (price_ref > price_max)
		price_ref = price_max;

	int y_ref = row_n - (price_ref - price_min) * y_step;
	if (y_ref < 0)
		y_ref = line_width - 1;
	if (y_ref >= row_n)
		y_ref = row_n - 1;

	for (int i = 0, x = 0; x < col_n; i++, x += dash_len) {
		if (i % 2 == 0) {
			for (int j = x; j < x + dash_len; j++) {
				for (int k = y_ref, c = 0; c < line_width; k--, c++) {
					int px = (k + y_min) * sc->width + (j + x_min);
					sc->fb[px / 8] |= (1ULL << (7 - px % 8));
				}
			}
		}
	}
}

static inline void gui_draw_panel_data(struct scrn *sc, char *s, int x)
{
	struct sprite sp;

	sp.offset_y = 415;
	sp.height = font_40_height;;

	for (sp.offset_x = x; *s; s++) {
		switch (*s) {
		case '0':
			sp.bmp = char_0_40_bmp;
			sp.width = char_num_40_width;
			break;
		case '1':
			sp.bmp = char_1_40_bmp;
			sp.width = char_num_40_width;
			break;
		case '2':
			sp.bmp = char_2_40_bmp;
			sp.width = char_num_40_width;
			break;
		case '3':
			sp.bmp = char_3_40_bmp;
			sp.width = char_num_40_width;
			break;
		case '4':
			sp.bmp = char_4_40_bmp;
			sp.width = char_num_40_width;
			break;
		case '5':
			sp.bmp = char_5_40_bmp;
			sp.width = char_num_40_width;
			break;
		case '6':
			sp.bmp = char_6_40_bmp;
			sp.width = char_num_40_width;
			break;
		case '7':
			sp.bmp = char_7_40_bmp;
			sp.width = char_num_40_width;
			break;
		case '8':
			sp.bmp = char_8_40_bmp;
			sp.width = char_num_40_width;
			break;
		case '9':
			sp.bmp = char_9_40_bmp;
			sp.width = char_num_40_width;
			break;
		case '-':
			sp.bmp = char_hyphen_40_bmp;
			sp.width = char_hyphen_40_width;
			break;
		case 'C':
			sp.bmp = char_C_40_bmp;
			sp.width = char_C_40_width;
			break;
		case '%':
			sp.bmp = char_percent_40_bmp;
			sp.width = char_percent_40_width;
			break;
		case '.':
			sp.bmp = char_period_40_bmp;
			sp.width = char_period_40_width;
			break;
		case 248:
			sp.bmp = char_degree_40_bmp;
			sp.width = char_degree_40_width;
			break;
		case 'J':
			sp.bmp = char_J_40_bmp;
			sp.width = char_J_40_width;
			break;
		case 'a':
			sp.bmp = char_a_40_bmp;
			sp.width = char_a_40_width;
			break;
		case 'n':
			sp.bmp = char_n_40_bmp;
			sp.width = char_n_40_width;
			break;
		case 'F':
			sp.bmp = char_F_40_bmp;
			sp.width = char_F_40_width;
			break;
		case 'e':
			sp.bmp = char_e_40_bmp;
			sp.width = char_e_40_width;
			break;
		case 'b':
			sp.bmp = char_b_40_bmp;
			sp.width = char_b_40_width;
			break;
		case 'M':
			sp.bmp = char_M_40_bmp;
			sp.width = char_M_40_width;
			break;
		case 'r':
			sp.bmp = char_r_40_bmp;
			sp.width = char_r_40_width;
			break;
		case 'A':
			sp.bmp = char_A_40_bmp;
			sp.width = char_A_40_width;
			break;
		case 'p':
			sp.bmp = char_p_40_bmp;
			sp.width = char_p_40_width;
			break;
		case 'y':
			sp.bmp = char_y_40_bmp;
			sp.width = char_y_40_width;
			break;
		case 'u':
			sp.bmp = char_u_40_bmp;
			sp.width = char_u_40_width;
			break;
		case 'l':
			sp.bmp = char_l_40_bmp;
			sp.width = char_l_40_width;
			break;
		case 'g':
			sp.bmp = char_g_40_bmp;
			sp.width = char_g_40_width;
			break;
		case 'S':
			sp.bmp = char_S_40_bmp;
			sp.width = char_S_40_width;
			break;
		case 'O':
			sp.bmp = char_O_40_bmp;
			sp.width = char_O_40_width;
			break;
		case 'c':
			sp.bmp = char_c_40_bmp;
			sp.width = char_c_40_width;
			break;
		case 't':
			sp.bmp = char_t_40_bmp;
			sp.width = char_t_40_width;
			break;
		case 'N':
			sp.bmp = char_N_40_bmp;
			sp.width = char_N_40_width;
			break;
		case 'o':
			sp.bmp = char_o_40_bmp;
			sp.width = char_o_40_width;
			break;
		case 'v':
			sp.bmp = char_v_40_bmp;
			sp.width = char_v_40_width;
			break;
		case 'D':
			sp.bmp = char_D_40_bmp;
			sp.width = char_D_40_width;
			break;
		case ':':
			sp.bmp = char_colon_40_bmp;
			sp.width = char_colon_40_width;
			break;
		case ' ':
			break;
		default:
			ESP_LOGE(TAG, "bottom panel: unknown character %c", *s);
			break;
		}

		if (*s != ' ') {
			scrn_draw(sc, &sp);
			sp.offset_x += sp.width;
		} else
			sp.offset_x += 15;
	}
}

void gui_draw_date(struct scrn *sc, struct tm *now)
{
	char s[13];

	if (strftime(s, sizeof(s), "%b %d %H:%M", now))
		gui_draw_panel_data(sc, s, 28);
	else
		ESP_LOGE(TAG, "strftime() failed for date");
}

void gui_draw_temp(struct scrn *sc)
{
	char s[7];

	dht_temp_get(s);
	gui_draw_panel_data(sc, s, 372);
}

void gui_draw_humid(struct scrn *sc)
{
	char s[6];

	dht_humid_get(s);
	gui_draw_panel_data(sc, s, 613);
}

void gui_draw_layout(struct scrn *sc)
{
	for (int i = 0; i < MAXLEN; i++)
		sc->fb[i] = layout_bmp[i];
}


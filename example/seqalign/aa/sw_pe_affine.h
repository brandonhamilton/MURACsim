/******************************************************************
	sw_pe_affine.h
*******************************************************************/

#ifndef _SW_PE_AFFINE_H
#define _SW_PE_AFFINE_H

#include "systemc.h"

template<unsigned LENGTH = 48, unsigned LOGLENGTH = 6>
SC_MODULE(sw_pe_affine) 
{
	//Local Parameters Declaration...
	static const unsigned SCORE_WIDTH = 11;
	static const unsigned N_A = 0;
	static const unsigned N_G = 1;
	static const unsigned N_T = 2;
	static const unsigned N_C = 3;
	static const unsigned INS_START = 3;
	static const unsigned INS_CONT = 1;
	static const unsigned DEL_START = 3;
	static const unsigned DEL_CONT = 1;
	static const unsigned TB_UP = 0;
	static const unsigned TB_DIAG = 1;
	static const unsigned TB_LEFT = 2;
	static const unsigned GOPEN = 12;
	static const unsigned GEXT = 4;

	//Interface Ports...
	sc_in<bool>	clk;
	sc_in<bool>	i_rst;
	sc_out<bool>	o_rst;
	sc_in<sc_uint<2> >	i_data;
	sc_in<sc_uint<2> >	i_preload;
	sc_in<sc_uint<SCORE_WIDTH> >	i_left_m;
	sc_in<sc_uint<SCORE_WIDTH> >	i_left_i;
	sc_in<sc_uint<SCORE_WIDTH> >	i_high;
	sc_in<bool>	i_vld;
	sc_in<bool>	i_local;
	sc_out<sc_uint<SCORE_WIDTH> >	o_right_m;
	sc_out<sc_uint<SCORE_WIDTH> >	o_right_i;
	sc_out<sc_uint<SCORE_WIDTH> >	o_high;
	sc_out<bool>	o_vld;
	sc_out<sc_uint<2> >	o_data;
	sc_in<bool>	start;

	//Internal Signals...
	sc_signal<sc_uint<2> >	state;
	sc_signal<sc_uint<SCORE_WIDTH> >	l_diag_score_m;
	sc_signal<sc_uint<SCORE_WIDTH> >	l_diag_score_i;
	sc_signal<sc_uint<SCORE_WIDTH> >	right_m_nxt;
	sc_signal<sc_uint<SCORE_WIDTH> >	right_i_nxt;
	sc_signal<sc_uint<SCORE_WIDTH> >	left_open;
	sc_signal<sc_uint<SCORE_WIDTH> >	left_ext;
	sc_signal<sc_uint<SCORE_WIDTH> >	up_open;
	sc_signal<sc_uint<SCORE_WIDTH> >	up_ext;
	sc_signal<sc_uint<SCORE_WIDTH> >	left_max;
	sc_signal<sc_uint<SCORE_WIDTH> >	up_max;
	sc_signal<sc_uint<SCORE_WIDTH> >	rightmax;
	sc_signal<sc_uint<SCORE_WIDTH> >	start_left;
	sc_signal<sc_uint<SCORE_WIDTH> >	match;
	sc_signal<sc_uint<SCORE_WIDTH> >	max_score_a;
	sc_signal<sc_uint<SCORE_WIDTH> >	max_score_b;
	sc_signal<sc_uint<SCORE_WIDTH> >	left_score;
	sc_signal<sc_uint<SCORE_WIDTH> >	up_score;
	sc_signal<sc_uint<SCORE_WIDTH> >	diag_score;
	sc_signal<sc_uint<SCORE_WIDTH> >	neutral_score;
	sc_signal<sc_uint<SCORE_WIDTH> >	left_score_b;

	//Processes Declaration...
	void assign_process_neutral_score_86();
	void assign_process_start_left_88();
	void assign_process_left_open_90();
	void assign_process_left_ext_91();
	void assign_process_up_open_92();
	void assign_process_up_ext_93();
	void assign_process_left_max_94();
	void assign_process_up_max_95();
	void assign_process_right_m_nxt_97();
	void assign_process_right_i_nxt_98();
	void always_process_103();
	void always_process_106();
	void always_process_128();
	void assign_process_rightmax_136();
	void always_process_139();
	void always_process_148();
	void always_process_166();

	//Constructor Declaration...
	SC_CTOR(sw_pe_affine)
	{
		SC_METHOD(assign_process_neutral_score_86);

		SC_METHOD(assign_process_start_left_88);
		sensitive << state << l_diag_score_m;

		SC_METHOD(assign_process_left_open_90);
		sensitive << i_left_m;

		SC_METHOD(assign_process_left_ext_91);
		sensitive << i_left_i;

		SC_METHOD(assign_process_up_open_92);
		sensitive << o_right_m;

		SC_METHOD(assign_process_up_ext_93);
		sensitive << o_right_i;

		SC_METHOD(assign_process_left_max_94);
		sensitive << start << start_left << left_open << left_ext;

		SC_METHOD(assign_process_up_max_95);
		sensitive << up_open << up_ext;

		SC_METHOD(assign_process_right_m_nxt_97);
		sensitive << match << l_diag_score_m << l_diag_score_i;

		SC_METHOD(assign_process_right_i_nxt_98);
		sensitive << left_max << up_max;

		SC_METHOD(always_process_103);
		sensitive << clk.pos();

		SC_METHOD(always_process_106);
		sensitive << i_data << i_preload;

		SC_METHOD(always_process_128);
		sensitive << clk.pos();

		SC_METHOD(assign_process_rightmax_136);
		sensitive << right_m_nxt << right_i_nxt;

		SC_METHOD(always_process_139);
		sensitive << clk.pos();

		SC_METHOD(always_process_148);
		sensitive << clk.pos();

		SC_METHOD(always_process_166);
		sensitive << clk.pos();
	}
};

template<unsigned LENGTH, unsigned LOGLENGTH>
void sw_pe_affine<LENGTH, LOGLENGTH>::assign_process_neutral_score_86() 
{
	neutral_score = 1024;
}

template<unsigned LENGTH, unsigned LOGLENGTH>
void sw_pe_affine<LENGTH, LOGLENGTH>::assign_process_start_left_88() 
{
	if( state.read() == 1 )
	{
		start_left = l_diag_score_m.read() - GOPEN;
	}
	else
	{
		start_left = l_diag_score_m.read() - GEXT;
	};
}

template<unsigned LENGTH, unsigned LOGLENGTH>
void sw_pe_affine<LENGTH, LOGLENGTH>::assign_process_left_open_90() 
{
	left_open = i_left_m.read() - GOPEN;
}

template<unsigned LENGTH, unsigned LOGLENGTH>
void sw_pe_affine<LENGTH, LOGLENGTH>::assign_process_left_ext_91() 
{
	left_ext = i_left_i.read() - GEXT;
}

template<unsigned LENGTH, unsigned LOGLENGTH>
void sw_pe_affine<LENGTH, LOGLENGTH>::assign_process_up_open_92() 
{
	up_open = o_right_m.read() - GOPEN;
}

template<unsigned LENGTH, unsigned LOGLENGTH>
void sw_pe_affine<LENGTH, LOGLENGTH>::assign_process_up_ext_93() 
{
	up_ext = o_right_i.read() - GEXT;
}

template<unsigned LENGTH, unsigned LOGLENGTH>
void sw_pe_affine<LENGTH, LOGLENGTH>::assign_process_left_max_94() 
{
	if( start.read() )
	{
		left_max = start_left.read();
	}
	else
	{
		if( left_open.read() > left_ext.read() )
		{
			left_max = left_open.read();
		}
		else
		{
			left_max = left_ext.read();
		};
	};
}

template<unsigned LENGTH, unsigned LOGLENGTH>
void sw_pe_affine<LENGTH, LOGLENGTH>::assign_process_up_max_95() 
{
	if( up_open.read() > up_ext.read() )
	{
		up_max = up_open.read();
	}
	else
	{
		up_max = up_ext.read();
	};
}

template<unsigned LENGTH, unsigned LOGLENGTH>
void sw_pe_affine<LENGTH, LOGLENGTH>::assign_process_right_m_nxt_97() 
{
	right_m_nxt = match.read() + (l_diag_score_m.read() > l_diag_score_i.read() ? l_diag_score_m.read() : l_diag_score_i.read());
}

template<unsigned LENGTH, unsigned LOGLENGTH>
void sw_pe_affine<LENGTH, LOGLENGTH>::assign_process_right_i_nxt_98() 
{
	if( left_max.read() > up_max.read() )
	{
		right_i_nxt = left_max.read();
	}
	else
	{
		right_i_nxt = up_max.read();
	};
}

template<unsigned LENGTH, unsigned LOGLENGTH>
void sw_pe_affine<LENGTH, LOGLENGTH>::always_process_103() 
{
	o_rst = i_rst.read();
}

template<unsigned LENGTH, unsigned LOGLENGTH>
void sw_pe_affine<LENGTH, LOGLENGTH>::always_process_106() 
{
	match = (i_data.read().range(1,0) == i_preload.read().range(1,0) ? 5 : 0x7fc);
}

template<unsigned LENGTH, unsigned LOGLENGTH>
void sw_pe_affine<LENGTH, LOGLENGTH>::always_process_128() 
{
	if( i_rst.read() == 1 ) 
	{
		o_vld = 0;
	}
	else
	{
		o_vld = i_vld.read();
	}
}

template<unsigned LENGTH, unsigned LOGLENGTH>
void sw_pe_affine<LENGTH, LOGLENGTH>::assign_process_rightmax_136() 
{
	if( right_m_nxt.read() > right_i_nxt.read() )
	{
		rightmax = right_m_nxt.read();
	}
	else
	{
		rightmax = right_i_nxt.read();
	};
}

template<unsigned LENGTH, unsigned LOGLENGTH>
void sw_pe_affine<LENGTH, LOGLENGTH>::always_process_139() 
{
	if( i_rst.read() == 1 ) 
	{
		o_high = neutral_score.read();
	}
	else
	{
		if( i_vld.read() == 1 ) 
		{
			if( o_high.read() > rightmax.read() )
			{
				if( o_high.read() > i_high.read() )
				{
					o_high = o_high.read();
				}
				else
				{
					o_high = i_high.read();
				};
			}
			else
			{
				if( rightmax.read() > i_high.read() )
				{
					o_high = rightmax.read();
				}
				else
				{
					o_high = i_high.read();
				};
			};
		}
	}
}

template<unsigned LENGTH, unsigned LOGLENGTH>
void sw_pe_affine<LENGTH, LOGLENGTH>::always_process_148() 
{
	if( i_rst.read() == 1 ) 
	{
		if( i_local.read() )
		{
			o_right_m = neutral_score.read();
		}
		else
		{
			o_right_m = i_left_m.read() - (start.read() ? GOPEN : GEXT);
		};
		if( i_local.read() )
		{
			o_right_i = neutral_score.read();
		}
		else
		{
			o_right_i = i_left_i.read() - (start.read() ? GOPEN : GEXT);
		};
		o_data = 0;
		if( start.read() )
		{
			l_diag_score_m = neutral_score.read();
		}
		else
		{
			if( i_local.read() )
			{
				l_diag_score_m = neutral_score.read();
			}
			else
			{
				l_diag_score_m = i_left_m.read();
			};
		};
		if( start.read() )
		{
			l_diag_score_i = neutral_score.read();
		}
		else
		{
			if( i_local.read() )
			{
				l_diag_score_i = neutral_score.read();
			}
			else
			{
				l_diag_score_i = i_left_i.read();
			};
		};
	}
	else
	{
		if( i_vld.read() == 1 ) 
		{
			o_data = i_data.read().range(1,0);
			if( i_local.read() )
			{
				if( right_m_nxt.read() > neutral_score.read() )
				{
					o_right_m = right_m_nxt.read();
				}
				else
				{
					o_right_m = neutral_score.read();
				};
			}
			else
			{
				o_right_m = right_m_nxt.read();
			};
			if( i_local.read() )
			{
				if( right_i_nxt.read() > neutral_score.read() )
				{
					o_right_i = right_i_nxt.read();
				}
				else
				{
					o_right_i = neutral_score.read();
				};
			}
			else
			{
				o_right_i = right_i_nxt.read();
			};
			if( start.read() )
			{
				if( i_local.read() )
				{
					l_diag_score_m = neutral_score.read();
				}
				else
				{
					l_diag_score_m = l_diag_score_m.read() - GEXT;
				};
			}
			else
			{
				l_diag_score_m = i_left_m.read();
			};
			if( start.read() )
			{
				if( i_local.read() )
				{
					l_diag_score_i = neutral_score.read();
				}
				else
				{
					l_diag_score_i = l_diag_score_i.read() - GEXT;
				};
			}
			else
			{
				l_diag_score_i = i_left_i.read();
			};
		}
	}
}

template<unsigned LENGTH, unsigned LOGLENGTH>
void sw_pe_affine<LENGTH, LOGLENGTH>::always_process_166() 
{
	if( i_rst.read() ) 
	{
		state = 0;
	}
	else
	{
		switch( state.read().range(1,0) )
		{
			case 0 : 
				state = 1;
				break;
			case 1 : 
				if( i_vld.read() == 1 ) 
				{
					state = 2;
				}
				break;
			case 2 : 
				if( i_vld.read() == 0 ) 
				{
					state = 1;
				}
				break;
			case 3 : 
				if( i_rst.read() ) 
				{
					state = 0;
				}
				break;
		}
	}
}

#endif

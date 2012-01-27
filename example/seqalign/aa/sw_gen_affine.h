/******************************************************************
	sw_gen_affine.h
	This file is generated from sw_gen_affine.v by V2SC
	(c) Copryight 2007 by Ali Haj Abutalebi & Leila Mahmoudi Ayough
	Mazdak and Alborz Design Automation
	email: info@mazdak-alborz.com
	website: www.mazdak-alborz.com
*******************************************************************/

#ifndef _SW_GEN_AFFINE_H
#define _SW_GEN_AFFINE_H

#include "systemc.h"
#include "sw_pe_affine.h"

template<unsigned LOGLENGTH = 6, unsigned LENGTH = 48>
SC_MODULE(sw_gen_affine) 
{
	//Local Parameters Declaration...
	static const unsigned SCORE_WIDTH = 11;
	static const unsigned N_A = 0;
	static const unsigned N_G = 1;
	static const unsigned N_T = 2;
	static const unsigned N_C = 3;
	static const unsigned INS = 1;
	static const unsigned DEL = 1;
	static const unsigned TB_UP = 0;
	static const unsigned TB_DIAG = 1;
	static const unsigned TB_LEFT = 2;

	//Interface Ports...
	sc_in<bool>	clk;
	sc_in<bool>	rst;
	sc_in<sc_uint<LOGLENGTH> >	i_query_length;
	sc_in<bool>	i_local;
	sc_in<sc_uint<(LENGTH * 2)> > query;
	sc_in<bool>	i_vld;
	sc_in<sc_uint<2> >	i_data;
	sc_out<bool>	o_vld;
	sc_out<sc_uint<SCORE_WIDTH> >	m_result;

	//Internal Signals...
	sc_signal<sc_uint<(2 * (LENGTH - 1)) + 1 + 1> >	data;
	sc_signal<bool>	vld[LENGTH];
	sc_signal<sc_uint<SCORE_WIDTH> >	right_m[LENGTH];
	sc_signal<sc_uint<SCORE_WIDTH> >	right_i[LENGTH];
	sc_signal<sc_uint<SCORE_WIDTH> >	high_score[LENGTH];
	sc_signal<sc_uint<LENGTH> >	gap;
	sc_signal<sc_uint<LENGTH> >	reset;

	//Processes Declaration...
	void assign_process_o_vld_50();
	void assign_process_m_result_51();

	// Processing elements
	sw_pe_affine<LENGTH, LOGLENGTH>* pe_block[LENGTH]; 

	//Local Temporary Signals...
	sc_signal<bool> reset_l[LENGTH];
	sc_signal<sc_uint<2> > query_l[LENGTH];
	sc_signal<sc_uint<11> > BINARY_10000000000;
	sc_signal<sc_uint<11> > BINARY_0;
	sc_signal<sc_uint<2> >  data_l[LENGTH];
	sc_signal<bool> ONE_SUPPLY;
	sc_signal<bool> ZERO_SUPPLY;

	//Signal Handler...
	void signal_handler()
	{
		for (int i=0; i < LENGTH; i = i + 1) {
			reset_l[i] = reset.read()[i];
			query_l[i] = query.read().range(i*2+1,i*2);

			if (i == 0) {
				data_l[i] = i_data.read().range(1,0);
			} else {
				data_l[i] = data.read().range(2*(i-1)+1,(i-1)*2);
			}
		}
		
		BINARY_10000000000 = 1024;
		BINARY_0 = 0;
		ONE_SUPPLY = 1;
		ZERO_SUPPLY = 0;
	}

	//Constructor Declaration...
	SC_CTOR(sw_gen_affine)  {

		SC_METHOD(assign_process_o_vld_50);
		sensitive << i_query_length;

		SC_METHOD(assign_process_m_result_51);
		sensitive << i_local << i_query_length;

		char *name[LENGTH];
		for (int i=0; i < LENGTH; i = i + 1) {
			name[i] = (char *)malloc(15);
			sprintf(name[i], "pe_block_%d", i);
			pe_block[i] = new sw_pe_affine<LENGTH, LOGLENGTH>(name[i]);
			pe_block[i]->clk(clk);
			if (i == 0) {
				pe_block[i]->i_rst(rst);
				pe_block[i]->i_data(i_data);
				pe_block[i]->i_left_m(BINARY_10000000000);
				pe_block[i]->i_left_i(BINARY_10000000000);
				pe_block[i]->i_vld(i_vld);
				pe_block[i]->i_high(BINARY_0);
				pe_block[i]->start(ONE_SUPPLY);
			} else {
				pe_block[i]->i_rst(reset_l[i-1]);
				pe_block[i]->i_data(data_l[i]);
				pe_block[i]->i_left_m(right_m[i-1]);
				pe_block[i]->i_left_i(right_i[i-1]);
				pe_block[i]->i_vld(vld[i-1]);
				pe_block[i]->i_high(high_score[i-1]);
				pe_block[i]->start(ZERO_SUPPLY);
			}
			
			pe_block[i]->o_rst(reset_l[i]);
			pe_block[i]->i_preload(query_l[i]);
			pe_block[i]->i_local(i_local);

			pe_block[i]->o_right_m(right_m[i]);
			pe_block[i]->o_right_i(right_i[i]);
			pe_block[i]->o_high(high_score[i]);
			pe_block[i]->o_vld(vld[i]);
			pe_block[i]->o_data(data_l[i]);
		}

		SC_METHOD(signal_handler);
		sensitive << reset << query << data;
	}
};

template<unsigned LOGLENGTH, unsigned LENGTH>
void sw_gen_affine<LOGLENGTH, LENGTH>::assign_process_o_vld_50() 
{
	o_vld = vld[i_query_length.read()].read();
}

template<unsigned LOGLENGTH, unsigned LENGTH>
void sw_gen_affine<LOGLENGTH, LENGTH>::assign_process_m_result_51() 
{
	if( i_local.read() )
	{
		m_result = high_score[i_query_length.read()].read();
	}
	else
	{
		if( right_m[i_query_length.read()].read() > right_i[i_query_length.read()].read() )
		{
			m_result = right_m[i_query_length.read()].read();
		}
		else
		{
			m_result = right_i[i_query_length.read()].read();
		};
	};
}

#endif


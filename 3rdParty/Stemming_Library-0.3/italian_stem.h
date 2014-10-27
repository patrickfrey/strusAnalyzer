/***************************************************************************
                          italian_stem.h  -  description
                             -------------------
    begin                : Sat May 18 2004
    copyright            : (C) 2004 by Blake Madden
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/

#ifndef __ITALIAN_STEM_H__
#define __ITALIAN_STEM_H__

namespace stemming
	{
	/**Italian can include the following accented forms: 
		-á é í ó ú à è ì ò ù 
	First, replace all acute accents by grave accents.
	And, as in French, put u after q, and u, i between vowels into upper case. The vowels are then 
		-a e i o u à è ì ò ù 
	R2 and RV have the same definition as in the Spanish stemmer.*/
	//------------------------------------------------------
	template<typename Tchar_type = char,
			typename Tchar_traits = std::char_traits<Tchar_type> >
	class italian_stem : stem<Tchar_type,Tchar_traits>
		{
	public:
		typedef stem<Tchar_type,Tchar_traits> Parent;

		//---------------------------------------------
		void operator()(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			if (text.length() < 3)
				{
				italian_acutes_to_graves(text);
				return;
				}

			//reset internal data
			Parent::m_r1 = Parent::m_r2 = Parent::m_rv = 0;

			trim_western_punctuation(text);
			italian_acutes_to_graves(text);
			hash_italian_ui(text, "aeiouàèìòùAEIOUÀÈÌÒÙ");

			find_r1(text, "aeiouàèìòùAEIOUÀÈÌÒÙ");
			find_r2(text, "aeiouàèìòùAEIOUÀÈÌÒÙ");
			find_spanish_rv(text, "aeiouàèìòùAEIOUÀÈÌÒÙ");

			//step 0:
			step_0(text);
			//step 1:
			size_t text_length = text.length();
			step_1(text);

			///step 2 is called only if step 1 did not remove a suffix
			if (text_length == text.length() )
				{
				step_2(text);
				}

			//step 3:
			step_3a(text);
			step_3b(text);

			unhash_italian_ui(text);
			}
	private:
		/**Search for the longest among the following suffixes 

			-ci gli la le li lo mi ne si ti vi sene gliela gliele glieli glielo gliene mela
			mele meli melo mene tela tele teli telo tene cela cele celi celo cene vela vele veli velo vene 

		following one of

			-#ando endo
			-#ar er ir 

		in RV. In case of (a) the suffix is deleted, in case (b) it is replace by e
		(guardandogli -> guardando, accomodarci -> accomodare)*/
		//---------------------------------------------
		void step_0(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			if (is_suffix(text,/*gliela*/'g', 'G', 'l', 'L', 'i', 'I', 'e', 'E', 'l', 'L', 'a', 'A') )
				{
				if (step_0a(text, 6) || step_0b(text, 6) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*gliele*/'g', 'G', 'l', 'L', 'i', 'I', 'e', 'E', 'l', 'L', 'e', 'E') )
				{
				if (step_0a(text, 6) || step_0b(text, 6) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*glieli*/'g', 'G', 'l', 'L', 'i', 'I', 'e', 'E', 'l', 'L', 'i', 'I') )
				{
				if (step_0a(text, 6) || step_0b(text, 6) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*glielo*/'g', 'G', 'l', 'L', 'i', 'I', 'e', 'E', 'l', 'L', 'o', 'O') )
				{
				if (step_0a(text, 6) || step_0b(text, 6) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*gliene*/'g', 'G', 'l', 'L', 'i', 'I', 'e', 'E', 'n', 'N', 'e', 'E') )
				{
				if (step_0a(text, 6) || step_0b(text, 6) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*sene*/'s', 'S', 'e', 'E', 'n', 'N', 'e', 'E') )
				{
				if (step_0a(text, 4) || step_0b(text, 4) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*mela*/'m', 'M', 'e', 'E', 'l', 'L', 'a', 'A') )
				{
				if (step_0a(text, 4) || step_0b(text, 4) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*mele*/'m', 'M', 'e', 'E', 'l', 'L', 'e', 'E') )
				{
				if (step_0a(text, 4) || step_0b(text, 4) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*meli*/'m', 'M', 'e', 'E', 'l', 'L', 'i', 'I') )
				{
				if (step_0a(text, 4) || step_0b(text, 4) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*melo*/'m', 'M', 'e', 'E', 'l', 'L', 'o', 'O') )
				{
				if (step_0a(text, 4) || step_0b(text, 4) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*mene*/'m', 'M', 'e', 'E', 'n', 'N', 'e', 'E') )
				{
				if (step_0a(text, 4) || step_0b(text, 4) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*tela*/'t', 'T', 'e', 'E', 'l', 'L', 'a', 'A') )
				{
				if (step_0a(text, 4) || step_0b(text, 4) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*tele*/'t', 'T', 'e', 'E', 'l', 'L', 'e', 'E') )
				{
				if (step_0a(text, 4) || step_0b(text, 4) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*teli*/'t', 'T', 'e', 'E', 'l', 'L', 'i', 'I') )
				{
				if (step_0a(text, 4) || step_0b(text, 4) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*telo*/'t', 'T', 'e', 'E', 'l', 'L', 'o', 'O') )
				{
				if (step_0a(text, 4) || step_0b(text, 4) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*tene*/'t', 'T', 'e', 'E', 'n', 'N', 'e', 'E') )
				{
				if (step_0a(text, 4) || step_0b(text, 4) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*cela*/'c', 'C', 'e', 'E', 'l', 'L', 'a', 'A') )
				{
				if (step_0a(text, 4) || step_0b(text, 4) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*cela*/'c', 'C', 'e', 'E', 'l', 'L', 'a', 'A') )
				{
				if (step_0a(text, 4) || step_0b(text, 4) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*celi*/'c', 'C', 'e', 'E', 'l', 'L', 'i', 'I') )
				{
				if (step_0a(text, 4) || step_0b(text, 4) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*celo*/'c', 'C', 'e', 'E', 'l', 'L', 'o', 'O') )
				{
				if (step_0a(text, 4) || step_0b(text, 4) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*cene*/'c', 'C', 'e', 'E', 'n', 'N', 'e', 'E') )
				{
				if (step_0a(text, 4) || step_0b(text, 4) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*vela*/'v', 'V', 'e', 'E', 'l', 'L', 'a', 'A') )
				{
				if (step_0a(text, 4) || step_0b(text, 4) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*vele*/'v', 'V', 'e', 'E', 'l', 'L', 'e', 'E') )
				{
				if (step_0a(text, 4) || step_0b(text, 4) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*veli*/'v', 'V', 'e', 'E', 'l', 'L', 'i', 'I') )
				{
				if (step_0a(text, 4) || step_0b(text, 4) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*velo*/'v', 'V', 'e', 'E', 'l', 'L', 'o', 'O') )
				{
				if (step_0a(text, 4) || step_0b(text, 4) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*vene*/'v', 'V', 'e', 'E', 'n', 'N', 'e', 'E') )
				{
				if (step_0a(text, 4) || step_0b(text, 4) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*gli*/'g', 'G', 'l', 'L', 'i', 'I') )
				{
				if (step_0a(text, 3) || step_0b(text, 3) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*ci*/'c', 'C', 'i', 'I') )
				{
				if (step_0a(text, 2) || step_0b(text, 2) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*la*/'l', 'L', 'a', 'A') )
				{
				if (step_0a(text, 2) || step_0b(text, 2) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*le*/'l', 'L', 'e', 'E') )
				{
				if (step_0a(text, 2) || step_0b(text, 2) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*li*/'l', 'L', 'i', 'I') )
				{
				if (step_0a(text, 2) || step_0b(text, 2) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*lo*/'l', 'L', 'o', 'O') )
				{
				if (step_0a(text, 2) || step_0b(text, 2) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*mi*/'m', 'M', 'i', 'I') )
				{
				if (step_0a(text, 2) || step_0b(text, 2) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*ne*/'n', 'N', 'e', 'E') )
				{
				if (step_0a(text, 2) || step_0b(text, 2) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*si*/'s', 'S', 'i', 'I') )
				{
				if (step_0a(text, 2) || step_0b(text, 2) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*ti*/'t', 'T', 'i', 'I') )
				{
				if (step_0a(text, 2) || step_0b(text, 2) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*vi*/'v', 'V', 'i', 'I') )
				{
				if (step_0a(text, 2) || step_0b(text, 2) ) { /*NOOP*/ }
				return;
				}
			}
		//---------------------------------------------
		bool step_0a(std::basic_string<Tchar_type, Tchar_traits>& text, size_t suffix_length)
			{
			if (text.length() >= suffix_length + 4 &&
				Parent::m_rv <= text.length()-4-suffix_length &&
				(/*ando*/(is_either(text[text.length()-4-suffix_length], 'a', 'A') &&
						is_either(text[text.length()-3-suffix_length], 'n', 'N') &&
						is_either(text[text.length()-2-suffix_length], 'd', 'D') &&
						is_either(text[text.length()-1-suffix_length], 'o', 'O') ) ||
				/*endo*/(is_either(text[text.length()-4-suffix_length], 'e', 'E') &&
						is_either(text[text.length()-3-suffix_length], 'n', 'N') &&
						is_either(text[text.length()-2-suffix_length], 'd', 'D') &&
						is_either(text[text.length()-1-suffix_length], 'o', 'O') ) ) )
				/*text.compare(text.length()-4-suffix_length, 4, "ando", 4) == 0 ||
				text.compare(text.length()-4-suffix_length, 4, "endo", 4) == 0*/
				{
				text.erase(text.end()-suffix_length, text.end() );
				update_r_sections(text);
				return true;
				}
			return false;
			}
		//---------------------------------------------
		bool step_0b(std::basic_string<Tchar_type, Tchar_traits>& text, size_t suffix_length)
			{
			if ((text.length() >= suffix_length + 2) &&
				Parent::m_rv <= text.length()-2-suffix_length &&
				(
				/*ar*/(is_either(text[text.length()-2-suffix_length], 'a', 'A') && is_either(text[text.length()-1-suffix_length], 'r', 'R') ) ||
				//BM text.compare(text.length()-2-suffix_length, 2, "ar", 2) == 0 ||
				/*er*/(is_either(text[text.length()-2-suffix_length], 'e', 'E') && is_either(text[text.length()-1-suffix_length], 'r', 'R') ) ||
				//BM text.compare(text.length()-2-suffix_length, 2, "er", 2) == 0 ||
				/*or*/(is_either(text[text.length()-2-suffix_length], 'i', 'I') && is_either(text[text.length()-1-suffix_length], 'r', 'R') )
				//BM text.compare(text.length()-2-suffix_length, 2, "ir", 2) == 0
				) )
				{
				text.replace(text.end()-suffix_length, text.end(), "e");
				update_r_sections(text);
				return true;
				}
			return false;
			}
		/**Search for the longest among the following suffixes, and perform the action indicated. 

			-anza   anze   ico   ici   ica   ice   iche   ichi   ismo   ismi   abile   abili   ibile   ibili   ista   iste   isti   istà   istè   istì   oso   osi   osa   ose   mente   atrice   atrici 
				-delete if in R2

			-azione   azioni   atore   atori delete if in R2 
				-if preceded by ic, delete if in R2

			-logia   logie 
				-replace with log if in R2

			-uzione   uzioni   usione   usioni 
				-replace with u if in R2

			-enza   enze 
				-replace with ente if in R2

			-amento   amenti   imento   imenti 
				-delete if in RV

			-amente 
				-delete if in R1 
				-if preceded by iv, delete if in R2 (and if further preceded by at, delete if in R2), otherwise, 
				-if preceded by os, ic or abil, delete if in R2

			-ità 
				-delete if in R2 
				-if preceded by abil, ic or iv, delete if in R2

			-ivo   ivi   iva   ive 
				-delete if in R2 
				-if preceded by at, delete if in R2 (and if further preceded by ic, delete if in R2)*/
		//---------------------------------------------
		void step_1(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			if (delete_if_is_in_rv(text,/*amento*/'a', 'A', 'm', 'M', 'e', 'E', 'n', 'N', 't', 'T', 'o', 'O') ||
				delete_if_is_in_rv(text,/*amenti*/'a', 'A', 'm', 'M', 'e', 'E', 'n', 'N', 't', 'T', 'i', 'I') ||
				delete_if_is_in_rv(text,/*imento*/'i', 'I', 'm', 'M', 'e', 'E', 'n', 'N', 't', 'T', 'o', 'O') ||
				delete_if_is_in_rv(text,/*imenti*/'i', 'I', 'm', 'M', 'e', 'E', 'n', 'N', 't', 'T', 'i', 'I') )
				{
				return;
				}
			else if (delete_if_is_in_r2(text,/*azione*/'a', 'A', 'z', 'Z', 'i', 'I', 'o', 'O', 'n', 'N', 'e', 'E') )
				{
				delete_if_is_in_r2(text,/*ic*/'i', 'I', 'c', 'C');
				return;
				}
			else if (delete_if_is_in_r2(text,/*azioni*/'a', 'A', 'z', 'Z', 'i', 'I', 'o', 'O', 'n', 'N', 'i', 'I') )
				{
				delete_if_is_in_r2(text,/*ic*/'i', 'I', 'c', 'C');
				return;
				}
			else if (is_suffix_in_r2(text,/*uzione*/'u', 'U', 'z', 'Z', 'i', 'I', 'o', 'O', 'n', 'N', 'e', 'E') ||
				is_suffix_in_r2(text,/*uzioni*/'u', 'U', 'z', 'Z', 'i', 'I', 'o', 'O', 'n', 'N', 'i', 'I') ||
				is_suffix_in_r2(text,/*usione*/'u', 'U', 's', 'S', 'i', 'I', 'o', 'O', 'n', 'N', 'e', 'E') ||
				is_suffix_in_r2(text,/*usioni*/'u', 'U', 's', 'S', 'i', 'I', 'o', 'O', 'n', 'N', 'i', 'I') )
				{
				text.erase(text.end()-5, text.end() );
				update_r_sections(text);
				return;
				}
			else if (delete_if_is_in_r1(text,/*amente*/'a', 'A', 'm', 'M', 'e', 'E', 'n', 'N', 't', 'T', 'e', 'E') )
				{
				if (delete_if_is_in_r2(text,/*iv*/'i', 'I', 'v', 'V') )
					{
					delete_if_is_in_r2(text,/*at*/'a', 'A', 't', 'T');
					}
				else if (delete_if_is_in_r2(text,/*abil*/'a', 'A', 'b', 'B', 'i', 'I', 'l', 'L') ||
						delete_if_is_in_r2(text,/*ic*/'i', 'I', 'c', 'C') ||
						delete_if_is_in_r2(text,/*os*/'o', 'O', 's', 'S') )
					{ /*NOOP*/ }
				return;
				}
			else if (delete_if_is_in_r2(text,/*atrice*/'a', 'A', 't', 'T', 'r', 'R', 'i', 'I', 'c', 'C', 'e', 'E') )
				{
				return;
				}
			else if (delete_if_is_in_r2(text,/*atrici*/'a', 'A', 't', 'T', 'r', 'R', 'i', 'I', 'c', 'C', 'i', 'I') )
				{
				return;
				}
			else if (delete_if_is_in_r2(text,/*abile*/'a', 'A', 'b', 'B', 'i', 'I', 'l', 'L', 'e', 'E') )
				{
				return;
				}
			else if (delete_if_is_in_r2(text,/*abili*/'a', 'A', 'b', 'B', 'i', 'I', 'l', 'L', 'i', 'I') )
				{
				return;
				}
			else if (delete_if_is_in_r2(text,/*ibile*/'i', 'I', 'b', 'B', 'i', 'I', 'l', 'L', 'e', 'E') )
				{
				return;
				}
			else if (delete_if_is_in_r2(text,/*ibili*/'i', 'I', 'b', 'B', 'i', 'I', 'l', 'L', 'i', 'I') )
				{
				return;
				}
			else if (delete_if_is_in_r2(text,/*mente*/'m', 'M', 'e', 'E', 'n', 'N', 't', 'T', 'e', 'E') )
				{
				return;
				}
			else if (delete_if_is_in_r2(text,/*atore*/'a', 'A', 't', 'T', 'o', 'O', 'r', 'R', 'e', 'E') )
				{
				delete_if_is_in_r2(text,/*ic*/'i', 'I', 'c', 'C');
				return;
				}
			else if (delete_if_is_in_r2(text,/*atori*/'a', 'A', 't', 'T', 'o', 'O', 'r', 'R', 'i', 'I') )
				{
				delete_if_is_in_r2(text,/*ic*/'i', 'I', 'c', 'C');
				return;
				}
			else if (is_suffix_in_r2(text,/*logia*/'l', 'L', 'o', 'O', 'g', 'G', 'i', 'I', 'a', 'A') ||
				is_suffix_in_r2(text,/*logie*/'l', 'L', 'o', 'O', 'g', 'G', 'i', 'I', 'e', 'E') )
				{
				text.erase(text.end()-2, text.end() );
				update_r_sections(text);
				return;
				}
			else if (is_suffix_in_r2(text,/*enza*/'e', 'E', 'n', 'N', 'z', 'Z', 'a', 'A') ||
				is_suffix_in_r2(text,/*enze*/'e', 'E', 'n', 'N', 'z', 'Z', 'e', 'E') )
				{
				text.replace(text.end()-2, text.end(), "te");
				update_r_sections(text);
				return;
				}
			else if (delete_if_is_in_r2(text,/*anza*/'a', 'A', 'n', 'N', 'z', 'Z', 'a', 'A') )
				{
				return;
				}
			else if (delete_if_is_in_r2(text,/*anze*/'a', 'A', 'n', 'N', 'z', 'Z', 'e', 'E') )
				{
				return;
				}
			else if (delete_if_is_in_r2(text,/*iche*/'i', 'I', 'c', 'C', 'h', 'H', 'e', 'E') )
				{
				return;
				}
			else if (delete_if_is_in_r2(text,/*ichi*/'i', 'I', 'c', 'C', 'h', 'H', 'i', 'I') )
				{
				return;
				}
			else if (delete_if_is_in_r2(text,/*ismo*/'i', 'I', 's', 'S', 'm', 'M', 'o', 'O') )
				{
				return;
				}
			else if (delete_if_is_in_r2(text,/*ismi*/'i', 'I', 's', 'S', 'm', 'M', 'i', 'I') )
				{
				return;
				}
			else if (delete_if_is_in_r2(text,/*ista*/'i', 'I', 's', 'S', 't', 'T', 'a', 'A') )
				{
				return;
				}
			else if (delete_if_is_in_r2(text,/*iste*/'i', 'I', 's', 'S', 't', 'T', 'e', 'E') )
				{
				return;
				}
			else if (delete_if_is_in_r2(text,/*isti*/'i', 'I', 's', 'S', 't', 'T', 'i', 'I') )
				{
				return;
				}
			else if (delete_if_is_in_r2(text,/*istà*/'i', 'I', 's', 'S', 't', 'T', 'à', 'À') )
				{
				return;
				}
			else if (delete_if_is_in_r2(text,/*istè*/'i', 'I', 's', 'S', 't', 'T', 'è', 'È') )
				{
				return;
				}
			else if (delete_if_is_in_r2(text,/*istì*/'i', 'I', 's', 'S', 't', 'T', 'ì', 'Ì') )
				{
				return;
				}
			else if (delete_if_is_in_r2(text,/*ico*/'i', 'I', 'c', 'C', 'o', 'O') )
				{
				return;
				}
			else if (delete_if_is_in_r2(text,/*ici*/'i', 'I', 'c', 'C', 'i', 'I') )
				{
				return;
				}
			else if (delete_if_is_in_r2(text,/*ica*/'i', 'I', 'c', 'C', 'a', 'A') )
				{
				return;
				}
			else if (delete_if_is_in_r2(text,/*ice*/'i', 'I', 'c', 'C', 'e', 'E') )
				{
				return;
				}
			else if (delete_if_is_in_r2(text,/*oso*/'o', 'O', 's', 'S', 'o', 'O') )
				{
				return;
				}
			else if (delete_if_is_in_r2(text,/*osi*/'o', 'O', 's', 'S', 'i', 'I') )
				{
				return;
				}
			else if (delete_if_is_in_r2(text,/*osa*/'o', 'O', 's', 'S', 'a', 'A') )
				{
				return;
				}
			else if (delete_if_is_in_r2(text,/*ose*/'o', 'O', 's', 'S', 'e', 'E') )
				{
				return;
				}
			else if (delete_if_is_in_r2(text,/*ità*/'i', 'I', 't', 'T', 'à', 'À') )
				{
				if (delete_if_is_in_r2(text,/*abil*/'a', 'A', 'b', 'B', 'i', 'I', 'l', 'L') ||
					delete_if_is_in_r2(text,/*ic*/'i', 'I', 'c', 'C') ||
					delete_if_is_in_r2(text,/*iv*/'i', 'I', 'v', 'V') )
					{ /*NOOP*/ }
				return;
				}
			else if (delete_if_is_in_r2(text,/*ivo*/'i', 'I', 'v', 'V', 'o', 'O') ||
				delete_if_is_in_r2(text,/*ivi*/'i', 'I', 'v', 'V', 'i', 'I') ||
				delete_if_is_in_r2(text,/*iva*/'i', 'I', 'v', 'V', 'a', 'A') ||
				delete_if_is_in_r2(text,/*ive*/'i', 'I', 'v', 'V', 'e', 'E') )
				{
				if (delete_if_is_in_r2(text,/*at*/'a', 'A', 't', 'T') )
					{
					delete_if_is_in_r2(text,/*ic*/'i', 'I', 'c', 'C');
					}
				return;
				}
			}
		/**Do step 2 if no ending was removed by step 1
		Search for the longest among the following suffixes in RV, and if found, delete. 

			-ammo ando ano are arono asse assero assi assimo ata ate ati ato
			ava avamo avano avate avi avo emmo enda ende endi endo erà erai
			eranno ere erebbe erebbero erei eremmo eremo ereste eresti erete
			erò erono essero ete eva evamo evano evate evi evo Yamo iamo immo 
			irà irai iranno ire irebbe irebbero irei iremmo iremo ireste iresti
			irete irò irono isca iscano isce isci isco iscono issero ita ite iti
			ito iva ivamo ivano ivate ivi ivo ono uta ute uti uto ar ir 

		Always do steps 3a and 3b. */
		//---------------------------------------------
		void step_2(std::basic_string<Tchar_type, Tchar_traits>& text)
			{			
			if (delete_if_is_in_rv(text,/*erebbero*/'e', 'E', 'r', 'R', 'e', 'E', 'b', 'B', 'b', 'B', 'e', 'E', 'r', 'R', 'o', 'O', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*irebbero*/'i', 'I', 'r', 'R', 'e', 'E', 'b', 'B', 'b', 'B', 'e', 'E', 'r', 'R', 'o', 'O', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*assero*/'a', 'A', 's', 'S', 's', 'S', 'e', 'E', 'r', 'R', 'o', 'O', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*assimo*/'a', 'A', 's', 'S', 's', 'S', 'i', 'I', 'm', 'M', 'o', 'O', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*eranno*/'e', 'E', 'r', 'R', 'a', 'A', 'n', 'N', 'n', 'N', 'o', 'O', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*erebbe*/'e', 'E', 'r', 'R', 'e', 'E', 'b', 'B', 'b', 'B', 'e', 'E', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*eremmo*/'e', 'E', 'r', 'R', 'e', 'E', 'm', 'M', 'm', 'M', 'o', 'O', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ereste*/'e', 'E', 'r', 'R', 'e', 'E', 's', 'S', 't', 'T', 'e', 'E', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*eresti*/'e', 'E', 'r', 'R', 'e', 'E', 's', 'S', 't', 'T', 'i', 'I', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*essero*/'e', 'E', 's', 'S', 's', 'S', 'e', 'E', 'r', 'R', 'o', 'O', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*iranno*/'i', 'I', 'r', 'R', 'a', 'A', 'n', 'N', 'n', 'N', 'o', 'O', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*irebbe*/'i', 'I', 'r', 'R', 'e', 'E', 'b', 'B', 'b', 'B', 'e', 'E', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*iremmo*/'i', 'I', 'r', 'R', 'e', 'E', 'm', 'M', 'm', 'M', 'o', 'O', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ireste*/'i', 'I', 'r', 'R', 'e', 'E', 's', 'S', 't', 'T', 'e', 'E', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*iresti*/'i', 'I', 'r', 'R', 'e', 'E', 's', 'S', 't', 'T', 'i', 'I', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*iscano*/'i', 'I', 's', 'S', 'c', 'C', 'a', 'A', 'n', 'N', 'o', 'O', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*iscono*/'i', 'I', 's', 'S', 'c', 'C', 'o', 'O', 'n', 'N', 'o', 'O', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*issero*/'i', 'I', 's', 'S', 's', 'S', 'e', 'E', 'r', 'R', 'o', 'O', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*arono*/'a', 'A', 'r', 'R', 'o', 'O', 'n', 'N', 'o', 'O', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*avamo*/'a', 'A', 'v', 'V', 'a', 'A', 'm', 'M', 'o', 'O', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*avano*/'a', 'A', 'v', 'V', 'a', 'A', 'n', 'N', 'o', 'O', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*avate*/'a', 'A', 'v', 'V', 'a', 'A', 't', 'T', 'e', 'E', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*eremo*/'e', 'E', 'r', 'R', 'e', 'E', 'm', 'M', 'o', 'O', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*erete*/'e', 'E', 'r', 'R', 'e', 'E', 't', 'T', 'e', 'E', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*erono*/'e', 'E', 'r', 'R', 'o', 'O', 'n', 'N', 'o', 'O', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*evamo*/'e', 'E', 'v', 'V', 'a', 'A', 'm', 'M', 'o', 'O', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*evano*/'e', 'E', 'v', 'V', 'a', 'A', 'n', 'N', 'o', 'O', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*evate*/'e', 'E', 'v', 'V', 'a', 'A', 't', 'T', 'e', 'E', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*iremo*/'i', 'I', 'r', 'R', 'e', 'E', 'm', 'M', 'o', 'O', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*irete*/'i', 'I', 'r', 'R', 'e', 'E', 't', 'T', 'e', 'E', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*irono*/'i', 'I', 'r', 'R', 'o', 'O', 'n', 'N', 'o', 'O', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ivamo*/'i', 'I', 'v', 'V', 'a', 'A', 'm', 'M', 'o', 'O', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ivano*/'i', 'I', 'v', 'V', 'a', 'A', 'n', 'N', 'o', 'O', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ivate*/'i', 'I', 'v', 'V', 'a', 'A', 't', 'T', 'e', 'E', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ammo*/'a', 'A', 'm', 'M', 'm', 'M', 'o', 'O', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ando*/'a', 'A', 'n', 'N', 'd', 'D', 'o', 'O', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text, /*asse*/'a', 'A', 's', 'S', 's', 'S', 'e', 'E', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*assi*/'a', 'A', 's', 'S', 's', 'S', 'i', 'I', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*emmo*/'e', 'E', 'm', 'M', 'm', 'M', 'o', 'O', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*enda*/'e', 'E', 'n', 'N', 'd', 'D', 'a', 'A', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ende*/'e', 'E', 'n', 'N', 'd', 'D', 'e', 'E', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*endi*/'e', 'E', 'n', 'N', 'd', 'D', 'i', 'I', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*endo*/'e', 'E', 'n', 'N', 'd', 'D', 'o', 'O', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*erai*/'e', 'E', 'r', 'R', 'a', 'A', 'i', 'I', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*erei*/'e', 'E', 'r', 'R', 'e', 'E', 'i', 'I', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*yamo*/'y', 'Y', 'a', 'A', 'm', 'M', 'o', 'O', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*iamo*/'i', 'I', 'a', 'A', 'm', 'M', 'o', 'O', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*immo*/'i', 'I', 'm', 'M', 'm', 'M', 'o', 'O', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*irai*/'i', 'I', 'r', 'R', 'a', 'A', 'i', 'I', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*irei*/'i', 'I', 'r', 'R', 'e', 'E', 'i', 'I', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*isca*/'i', 'I', 's', 'S', 'c', 'C', 'a', 'A', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*isce*/'i', 'I', 's', 'S', 'c', 'C', 'e', 'E', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*isci*/'i', 'I', 's', 'S', 'c', 'C', 'i', 'I', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*isco*/'i', 'I', 's', 'S', 'c', 'C', 'o', 'O', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ano*/'a', 'A', 'n', 'N', 'o', 'O', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*are*/'a', 'A', 'r', 'R', 'e', 'E', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ata*/'a', 'A', 't', 'T', 'a', 'A', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ate*/'a', 'A', 't', 'T', 'e', 'E', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ati*/'a', 'A', 't', 'T', 'i', 'I', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ato*/'a', 'A', 't', 'T', 'o', 'O', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ava*/'a', 'A', 'v', 'V', 'a', 'A', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*avi*/'a', 'A', 'v', 'V', 'i', 'I', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*avo*/'a', 'A', 'v', 'V', 'o', 'O', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*erà*/'e', 'E', 'r', 'R', 'à', 'À', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ere*/'e', 'E', 'r', 'R', 'e', 'E', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*erò*/'e', 'E', 'r', 'R', 'ò', 'Ò', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ete*/'e', 'E', 't', 'T', 'e', 'E', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*eva*/'e', 'E', 'v', 'V', 'a', 'A', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*evi*/'e', 'E', 'v', 'V', 'i', 'I', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*evo*/'e', 'E', 'v', 'V', 'o', 'O', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*irà*/'i', 'I', 'r', 'R', 'à', 'À', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ire*/'i', 'I', 'r', 'R', 'e', 'E', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*irò*/'i', 'I', 'r', 'R', 'ò', 'Ò', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ita*/'i', 'I', 't', 'T', 'a', 'A', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ite*/'i', 'I', 't', 'T', 'e', 'E', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*iti*/'i', 'I', 't', 'T', 'i', 'I', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ito*/'i', 'I', 't', 'T', 'o', 'O', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*iva*/'i', 'I', 'v', 'V', 'a', 'A', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ivi*/'i', 'I', 'v', 'V', 'i', 'I', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ivo*/'i', 'I', 'v', 'V', 'o', 'O', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ono*/'o', 'O', 'n', 'N', 'o', 'O', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*uta*/'u', 'U', 't', 'T', 'a', 'A', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ute*/'u', 'U', 't', 'T', 'e', 'E', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*uti*/'u', 'U', 't', 'T', 'i', 'I', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*uto*/'u', 'U', 't', 'T', 'o', 'O', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ar*/'a', 'A', 'r', 'R', false) )
				{
				return;
				}
			///'ir' not in original specification, but used in general implementation
			else if (delete_if_is_in_rv(text,/*ir*/'i', 'I', 'r', 'R', false) )
				{
				return;
				}
			/**deletion or 'er' from rv is considered problematic,
			but part of the standard*/
			}
		/**Delete a final a, e, i, o, à, è, ì or ò if it is in RV,
		and a preceding i if it is in RV (crocchi -> crocch, crocchio -> crocch)*/
		//---------------------------------------------
		void step_3a(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			if (Parent::m_rv <= text.length()-1 &&
				string_util::is_one_of(text[text.length()-1], "aeioàèìòAEIOÀÈÌÒ") )
				{
				text.erase(text.end()-1, text.end() );
				update_r_sections(text);
				if (Parent::m_rv <= text.length()-1 &&					
					is_either(text[text.length()-1], 'i', 'I') )
					{
					text.erase(text.end()-1, text.end() );
					update_r_sections(text);
					}
				}
			}
		///Replace final ch (or gh) with c (or g) if in RV (crocch -> crocc) 
		//---------------------------------------------
		void step_3b(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			if (is_suffix_in_rv(text,/*ch*/'c', 'C', 'h', 'H') ||
				is_suffix_in_rv(text,/*gh*/'g', 'G', 'h', 'H') )
				{
				text.erase(text.end()-1, text.end() );
				update_r_sections(text);
				}
			}
		};
	}

#endif //__ITALIAN_STEM_H__

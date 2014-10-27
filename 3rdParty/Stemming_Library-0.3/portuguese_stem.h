/***************************************************************************
                          portuguese_stem.h  -  description
                             -------------------
    begin                : Sat May 17 2004
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

#ifndef __PORTUGUESE_STEM_H__
#define __PORTUGUESE_STEM_H__

#include "stemming.h"

namespace stemming
	{
	/**Letters in Portuguese include the following accented forms,

		-á é í ó ú â ê ô ç ã õ ü ñ

	The following letters are vowels: 
		-a e i o u á é í ó ú â ê ô

	And the two nasalised vowel forms
	
		-ã õ

	should be treated as a vowel followed by a consonant. 

	ã and õ are therefore replaced by a~ and o~ in the word, where ~ is a separate character to be treated as a consonant.
	And then R2 and RV have the same definition as in the Spanish stemmer.*/
	//------------------------------------------------------
	template<typename Tchar_type = char,
			typename Tchar_traits = std::char_traits<Tchar_type> >
	class portuguese_stem : stem<Tchar_type,Tchar_traits>
		{
	public:
		typedef stem<Tchar_type,Tchar_traits> Parent;

		portuguese_stem() : m_step1_step2_altered(false), m_altered_suffix_index(0)
			{}
		//---------------------------------------------
		void operator()(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			if (text.length() < 3)
				{
				return;
				}
			trim_western_punctuation(text);

			//reset internal data
			m_altered_suffix_index = 0;
			m_step1_step2_altered = false;
			Parent::m_r1 = Parent::m_r2 = Parent::m_rv =0;

			string_util::replace_all(text, "ã", "a~");
			string_util::replace_all(text, "Ã", "A~");
			string_util::replace_all(text, "õ", "o~");
			string_util::replace_all(text, "Õ", "O~");

			find_r1(text, "aeiouáéíóúâêôEIOUÁÉÍÓÚÂÊÔ");
			find_r2(text, "aeiouáéíóúâêôEIOUÁÉÍÓÚÂÊÔ");
			find_spanish_rv(text, "aeiouáéíóúâêôAEIOUÁÉÍÓÚÂÊÔ");

			step_1(text);
			//intermediate steps handled by step 1
			if (!m_step1_step2_altered)
				{
				step_4(text);
				}
			step_5(text);

			///Turn a~, o~ back into ã, õ 
			string_util::replace_all(text, "a~", "ã");
			string_util::replace_all(text, "A~", "Ã");
			string_util::replace_all(text, "o~", "õ");
			string_util::replace_all(text, "O~", "Õ");
			}
	private:
		/**Search for the longest among the following suffixes, and perform the action indicated. 

			-eza ezas ico ica icos icas ismo ismos ável ível ista istas oso osa osos osas amento amentos imento imentos adora ador aça~o adoras adores aço~es 
				-delete if in R2 

			-logía logías 
				-replace with log if in R2 

			-ución uciones 
				-replace with u if in R2 

			-ência ências 
				-replace with ente if in R2 

			-amente 
				-delete if in R1 
				-if preceded by iv, delete if in R2 (and if further preceded by at, delete if in R2), otherwise, 
				-if preceded by os, ic or ad, delete if in R2 

			-mente 
				-delete if in R2 
				-if preceded by avel or ível, delete if in R2 

			-idade idades 
				-delete if in R2 
				-if preceded by abil, ic or iv, delete if in R2 

				-iva ivo ivas ivos 
				-delete if in R2 
				-if preceded by at, delete if in R2 

			-ira iras 
				-replace with ir if in RV and preceded by e*/
		//---------------------------------------------
		void step_1(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			size_t original_length = text.length();
			if (is_suffix_in_r2(text,/*uciones*/'u', 'U', 'c', 'C', 'i', 'I', 'o', 'O', 'n', 'N', 'e', 'E', 's', 'S') )
				{
				text.erase(text.end()-6, text.end() );
				m_altered_suffix_index = text.length()-1;
				update_r_sections(text);
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*amentos*/'a', 'A', 'm', 'M', 'e', 'E', 'n', 'N', 't', 'T', 'o', 'O', 's', 'S') )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*imentos*/'i', 'I', 'm', 'M', 'e', 'E', 'n', 'N', 't', 'T', 'o', 'O', 's', 'S') )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*amento*/'a', 'A', 'm', 'M', 'e', 'E', 'n', 'N', 't', 'T', 'o', 'O') )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*imento*/'i', 'I', 'm', 'M', 'e', 'E', 'n', 'N', 't', 'T', 'o', 'O') )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*adoras*/'a', 'A', 'd', 'D', 'o', 'O', 'r', 'R', 'a', 'A', 's', 'S') )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*adores*/'a', 'A', 'd', 'D', 'o', 'O', 'r', 'R', 'e', 'E', 's', 'S') )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*aço~es*/'a', 'A', 'ç', 'Ç', 'o', 'O', '~', '~', 'e', 'E', 's', 'S') )
				{
				//NOOP (fall through to branching statement)
				}
			else if (is_suffix_in_r2(text,/*ências*/'ê', 'Ê', 'n', 'N', 'c', 'C', 'i', 'I', 'a', 'A', 's', 'S') )
				{
				///todo make more case sensitive
				text.replace(text.end()-6, text.end(), "ente");
				m_altered_suffix_index = text.length()-4;
				update_r_sections(text);
				//NOOP (fall through to branching statement)
				}
			else if (is_suffix_in_r2(text,/*logías*/'l', 'L', 'o', 'O', 'g', 'G', 'í', 'Í', 'a', 'A', 's', 'S') )
				{
				text.erase(text.end()-3, text.end() );
				m_altered_suffix_index = text.length()-3;
				update_r_sections(text);
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r1(text,/*amente*/'a', 'A', 'm', 'M', 'e', 'E', 'n', 'N', 't', 'T', 'e', 'E') )
				{
				if (delete_if_is_in_r2(text,/*iv*/'i', 'I', 'v', 'V', false) )
					{
					delete_if_is_in_r2(text,/*at*/'a', 'A', 't', 'T', false);
					}
				else
					{
					if (delete_if_is_in_r2(text,/*os*/'o', 'O', 's', 'S') ||
						delete_if_is_in_r2(text,/*ic*/'i', 'I', 'c', 'C') ||
						delete_if_is_in_r2(text,/*ad*/'a', 'A', 'd', 'D') )
						{
						//NOOP (fall through to branching statement)
						}
					}
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*idades*/'i', 'I', 'd', 'D', 'a', 'A', 'd', 'D', 'e', 'E', 's', 'S') )
				{
				if (delete_if_is_in_r2(text,/*abil*/'a', 'A', 'b', 'B', 'i', 'I', 'l', 'L') ||
					delete_if_is_in_r2(text,/*ic*/'i', 'I', 'c', 'C') ||
					delete_if_is_in_r2(text,/*iv*/'i', 'I', 'v', 'V') )
					{
					//NOOP (fall through to branching statement)
					}
				//NOOP (fall through to branching statement)
				}	
			else if (is_suffix_in_r2(text,/*logía*/'l', 'L', 'o', 'O', 'g', 'G', 'í', 'Í', 'a', 'A') )
				{
				text.erase(text.end()-2, text.end() );
				m_altered_suffix_index = text.length()-3;
				update_r_sections(text);
				//NOOP (fall through to branching statement)
				}	
			else if (is_suffix_in_r2(text,/*ución*/'u', 'U', 'c', 'C', 'i', 'I', 'ó', 'Ó', 'n', 'N') )
				{
				text.erase(text.end()-4, text.end() );
				m_altered_suffix_index = text.length()-1;
				update_r_sections(text);
				//NOOP (fall through to branching statement)
				}
			else if (is_suffix_in_r2(text,/*ência*/'ê', 'Ê', 'n', 'N', 'c', 'C', 'i', 'I', 'a', 'A') )
				{
				///make more case sensitive
				text.replace(text.end()-5, text.end(), "ente");
				m_altered_suffix_index = text.length()-4;
				update_r_sections(text);
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*mente*/'m', 'M', 'e', 'E', 'n', 'N', 't', 'T', 'e', 'E') )
				{
				if (delete_if_is_in_r2(text,/*avel*/'a', 'A', 'v', 'V', 'e', 'E', 'l', 'L') ||
					delete_if_is_in_r2(text,/*ível*/'í', 'Í', 'v', 'V', 'e', 'E', 'l', 'L') )
					{
					//NOOP (fall through to branching statement)
					}
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*idade*/'i', 'I', 'd', 'D', 'a', 'A', 'd', 'D', 'e', 'E') )
				{
				if (delete_if_is_in_r2(text,/*abil*/'a', 'A', 'b', 'B', 'i', 'I', 'l', 'L') ||
					delete_if_is_in_r2(text,/*ic*/'i', 'I', 'c', 'C') ||
					delete_if_is_in_r2(text,/*iv*/'i', 'I', 'v', 'V') )
					{
					//NOOP (fall through to branching statement)
					}
				//NOOP (fall through to branching statement)
				}
			else if (is_suffix(text,/*eiras*/'e', 'E', 'i', 'I', 'r', 'R', 'a', 'A', 's', 'S') )
				{
				if (Parent::m_rv <= static_cast<int>(text.length()-4) )
					{
					text.erase(text.end()-2, text.end() );
					m_altered_suffix_index = text.length()-3;
					update_r_sections(text);
					}
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*ismos*/'i', 'I', 's', 'S', 'm', 'M', 'o', 'O', 's', 'S') )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*istas*/'i', 'I', 's', 'S', 't', 'T', 'a', 'A', 's', 'S') )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*adora*/'a', 'A', 'd', 'D', 'o', 'O', 'r', 'R', 'a', 'A') )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*aça~o*/'a', 'A', 'ç', 'Ç', 'a', 'A', '~', '~', 'o', 'O') )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*ezas*/'e', 'E', 'z', 'Z', 'a', 'A', 's', 'S') )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*icos*/'i', 'I', 'c', 'C', 'o', 'O', 's', 'S') )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*icas*/'i', 'I', 'c', 'C', 'a', 'A', 's', 'S') )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*ismo*/'i', 'I', 's', 'S', 'm', 'M', 'o', 'O') )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*ável*/'á', 'Á', 'v', 'V', 'e', 'E', 'l', 'L') )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*ível*/'í', 'Í', 'v', 'V', 'e', 'E', 'l', 'L') )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*ista*/'i', 'I', 's', 'S', 't', 'T', 'a', 'A') )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*osos*/'o', 'O', 's', 'S', 'o', 'O', 's', 'S') )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*osas*/'o', 'O', 's', 'S', 'a', 'A', 's', 'S') )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*ador*/'a', 'A', 'd', 'D', 'o', 'O', 'r', 'R') )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*ivas*/'i', 'I', 'v', 'V', 'a', 'A', 's', 'S') )
				{
				if (delete_if_is_in_r2(text,/*at*/'a', 'A', 't', 'T') )
					{
					//NOOP (fall through to branching statement)
					}
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*ivos*/'i', 'I', 'v', 'V', 'o', 'O', 's', 'S') )
				{
				if (delete_if_is_in_r2(text,/*at*/'a', 'A', 't', 'T') )
					{
					//NOOP (fall through to branching statement)
					}
				//NOOP (fall through to branching statement)
				}
			else if (is_suffix(text,/*eira*/'e', 'E', 'i', 'I', 'r', 'R', 'a', 'A') )
				{
				if (Parent::m_rv <= static_cast<int>(text.length()-3) )
					{
					text.erase(text.end()-1, text.end() );
					m_altered_suffix_index = text.length()-3;
					update_r_sections(text);
					}
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*iva*/'i', 'I', 'v', 'V', 'a', 'A') )
				{
				if (delete_if_is_in_r2(text,/*at*/'a', 'A', 't', 'T') )
					{
					//NOOP (fall through to branching statement)
					}
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*ivo*/'i', 'I', 'v', 'V', 'o', 'O') )
				{
				if (delete_if_is_in_r2(text,/*at*/'a', 'A', 't', 'T') )
					{
					//NOOP (fall through to branching statement)
					}
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*eza*/'e', 'E', 'z', 'Z', 'a', 'A') )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*ico*/'i', 'I', 'c', 'C', 'o', 'O') )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*ica*/'i', 'I', 'c', 'C', 'a', 'A') )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*oso*/'o', 'O', 's', 'S', 'o', 'O') )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*osa*/'o', 'O', 's', 'S', 'a', 'A') )
				{
				//NOOP (fall through to branching statement)
				}
			//branch to the next appropriate step
			if (original_length == text.length() )
				{
				step_2(text);
				}
			else
				{
				m_step1_step2_altered = true;
				step_3(text);
				}
 			}
		/**Do step 2 if no ending was removed by step 1.
		Search for the longest among the following suffixes in RV, and if found, delete. 

			-ada ida ia aria eria iria ará ara erá era irá ava asse esse isse aste este iste
			ei arei erei irei am iam ariam eriam iriam aram eram iram avam em arem erem irem
			assem essem issem ado ido ando endo indo ara~o era~o ira~o ar er ir as adas idas
			ias arias erias irias arás aras erás eras irás avas es ardes erdes irdes ares eres
			ires asses esses isses astes estes istes is ais eis íeis aríeis eríeis iríeis áreis
			areis éreis ereis íreis ireis ásseis ésseis ísseis áveis ados idos ámos amos íamos
			aríamos eríamos iríamos áramos éramos íramos ávamos emos aremos eremos iremos ássemos
			êssemos íssemos imos armos ermos irmos eu iu ou ira iras

		If the last step to be obeyed - either step 1 or 2 - altered the word, do step 3*/
		//---------------------------------------------
		void step_2(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			size_t original_length = text.length();
			
			if (delete_if_is_in_rv(text,/*aríamos*/'a', 'A', 'r', 'R', 'í', 'Í', 'a', 'A', 'm', 'M', 'o', 'O', 's', 'S', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*eríamos*/'e', 'E', 'r', 'R', 'í', 'Í', 'a', 'A', 'm', 'M', 'o', 'O', 's', 'S', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*iríamos*/'i', 'I', 'r', 'R', 'í', 'Í', 'a', 'A', 'm', 'M', 'o', 'O', 's', 'S', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ássemos*/'á', 'Á', 's', 'S', 's', 'S', 'e', 'E', 'm', 'M', 'o', 'O', 's', 'S', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*êssemos*/'ê', 'Ê', 's', 'S', 's', 'S', 'e', 'E', 'm', 'M', 'o', 'O', 's', 'S', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*íssemos*/'í', 'Í', 's', 'S', 's', 'S', 'e', 'E', 'm', 'M', 'o', 'O', 's', 'S', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*aríeis*/'a', 'A', 'r', 'R', 'í', 'Í', 'e', 'E', 'i', 'I', 's', 'S', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*eríeis*/'e', 'E', 'r', 'R', 'í', 'Í', 'e', 'E', 'i', 'I', 's', 'S', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*iríeis*/'i', 'I', 'r', 'R', 'í', 'Í', 'e', 'E', 'i', 'I', 's', 'S', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ásseis*/'á', 'Á', 's', 'S', 's', 'S', 'e', 'E', 'i', 'I', 's', 'S', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ésseis*/'é', 'É', 's', 'S', 's', 'S', 'e', 'E', 'i', 'I', 's', 'S', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ísseis*/'í', 'Í', 's', 'S', 's', 'S', 'e', 'E', 'i', 'I', 's', 'S', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*áramos*/'á', 'Á', 'r', 'R', 'a', 'A', 'm', 'M', 'o', 'O', 's', 'S', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*éramos*/'é', 'É', 'r', 'R', 'a', 'A', 'm', 'M', 'o', 'O', 's', 'S', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*íramos*/'í', 'Í', 'r', 'R', 'a', 'A', 'm', 'M', 'o', 'O', 's', 'S', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ávamos*/'á', 'Á', 'v', 'V', 'a', 'A', 'm', 'M', 'o', 'O', 's', 'S', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*aremos*/'a', 'A', 'r', 'R', 'e', 'E', 'm', 'M', 'o', 'O', 's', 'S', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*eremos*/'e', 'E', 'r', 'R', 'e', 'E', 'm', 'M', 'o', 'O', 's', 'S', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*iremos*/'i', 'I', 'r', 'R', 'e', 'E', 'm', 'M', 'o', 'O', 's', 'S' ,false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ariam*/'a', 'A', 'r', 'R', 'i', 'I', 'a', 'A', 'm', 'M', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*eriam*/'e', 'E', 'r', 'R', 'i', 'I', 'a', 'A', 'm', 'M', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*iriam*/'i', 'I', 'r', 'R', 'i', 'I', 'a', 'A', 'm', 'M', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*assem*/'a', 'A', 's', 'S', 's', 'S', 'e', 'E', 'm', 'M', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*essem*/'e', 'E', 's', 'S', 's', 'S', 'e', 'E', 'm', 'M', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*issem*/'i', 'I', 's', 'S', 's', 'S', 'e', 'E', 'm', 'M',false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ara~o*/'a', 'A', 'r', 'R', 'a', 'A', '~', '~', 'o', 'O',false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*era~o*/'e', 'E', 'r', 'R', 'a', 'A', '~', '~', 'o', 'O',false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ira~o*/'i', 'I', 'r', 'R', 'a', 'A', '~', '~', 'o', 'O',false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*arias*/'a', 'A', 'r', 'R', 'i', 'I', 'a', 'A', 's', 'S',false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*erias*/'e', 'E', 'r', 'R', 'i', 'I', 'a', 'A', 's', 'S',false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*irias*/'i', 'I', 'r', 'R', 'i', 'I', 'a', 'A', 's', 'S',false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ardes*/'a', 'A', 'r', 'R', 'd', 'D', 'e', 'E', 's', 'S',false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*erdes*/'e', 'E', 'r', 'R', 'd', 'D', 'e', 'E', 's', 'S',false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*irdes*/'i', 'I', 'r', 'R', 'd', 'D', 'e', 'E', 's', 'S',false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*asses*/'a', 'A', 's', 'S', 's', 'S', 'e', 'E', 's', 'S',false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*esses*/'e', 'E', 's', 'S', 's', 'S', 'e', 'E', 's', 'S',false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*isses*/'i', 'I', 's', 'S', 's', 'S', 'e', 'E', 's', 'S',false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*astes*/'a', 'A', 's', 'S', 't', 'T', 'e', 'E', 's', 'S',false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*estes*/'e', 'E', 's', 'S', 't', 'T', 'e', 'E', 's', 'S',false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*istes*/'i', 'I', 's', 'S', 't', 'T', 'e', 'E', 's', 'S',false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*áreis*/'á', 'Á', 'r', 'R', 'e', 'E', 'i', 'I', 's', 'S',false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*areis*/'a', 'A', 'r', 'R', 'e', 'E', 'i', 'I', 's', 'S',false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*éreis*/'é', 'É', 'r', 'R', 'e', 'E', 'i', 'I', 's', 'S',false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ereis*/'e', 'E', 'r', 'R', 'e', 'E', 'i', 'I', 's', 'S',false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*íreis*/'í', 'Í', 'r', 'R', 'e', 'E', 'i', 'I', 's', 'S',false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ireis*/'i', 'I', 'r', 'R', 'e', 'E', 'i', 'I', 's', 'S',false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*áveis*/'á', 'Á', 'v', 'V', 'e', 'E', 'i', 'I', 's', 'S',false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*íamos*/'í', 'Í', 'a', 'A', 'm', 'M', 'o', 'O', 's', 'S',false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*armos*/'a', 'A', 'r', 'R', 'm', 'M', 'o', 'O', 's', 'S',false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ermos*/'e', 'E', 'r', 'R', 'm', 'M', 'o', 'O', 's', 'S',false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*irmos*/'i', 'I', 'r', 'R', 'm', 'M', 'o', 'O', 's', 'S',false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*aria*/'a', 'A', 'r', 'R', 'i', 'I', 'a', 'A', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*eria*/'e', 'E', 'r', 'R', 'i', 'I', 'a', 'A', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*iria*/'i', 'I', 'r', 'R', 'i', 'I', 'a', 'A', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*asse*/'a', 'A', 's', 'S', 's', 'S', 'e', 'E', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*esse*/'e', 'E', 's', 'S', 's', 'S', 'e', 'E', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*isse*/'i', 'I', 's', 'S', 's', 'S', 'e', 'E', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*aste*/'a', 'A', 's', 'S', 't', 'T', 'e', 'E', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*este*/'e', 'E', 's', 'S', 't', 'T', 'e', 'E', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*iste*/'i', 'I', 's', 'S', 't', 'T', 'e', 'E', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*arei*/'a', 'A', 'r', 'R', 'e', 'E', 'i', 'I', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*erei*/'e', 'E', 'r', 'R', 'e', 'E', 'i', 'I', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*irei*/'i', 'I', 'r', 'R', 'e', 'E', 'i', 'I', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*aram*/'a', 'A', 'r', 'R', 'a', 'A', 'm', 'M', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*eram*/'e', 'E', 'r', 'R', 'a', 'A', 'm', 'M', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*iram*/'i', 'I', 'r', 'R', 'a', 'A', 'm', 'M', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*avam*/'a', 'A', 'v', 'V', 'a', 'A', 'm', 'M', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*arem*/'a', 'A', 'r', 'R', 'e', 'E', 'm', 'M', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*erem*/'e', 'E', 'r', 'R', 'e', 'E', 'm', 'M', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*irem*/'i', 'I', 'r', 'R', 'e', 'E', 'm', 'M', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ando*/'a', 'A', 'n', 'N', 'd', 'D', 'o', 'O', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*endo*/'e', 'E', 'n', 'N', 'd', 'D', 'o', 'O', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*indo*/'i', 'I', 'n', 'N', 'd', 'D', 'o', 'O', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*adas*/'a', 'A', 'd', 'D', 'a', 'A', 's', 'S', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*idas*/'i', 'I', 'd', 'D', 'a', 'A', 's', 'S', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*arás*/'a', 'A', 'r', 'R', 'á', 'Á', 's', 'S', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*aras*/'a', 'A', 'r', 'R', 'a', 'A', 's', 'S', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*erás*/'e', 'E', 'r', 'R', 'á', 'Á', 's', 'S', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*eras*/'e', 'E', 'r', 'R', 'a', 'A', 's', 'S', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*irás*/'i', 'I', 'r', 'R', 'á', 'Á', 's', 'S', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*avas*/'a', 'A', 'v', 'V', 'a', 'A', 's', 'S', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ares*/'a', 'A', 'r', 'R', 'e', 'E', 's', 'S', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*eres*/'e', 'E', 'r', 'R', 'e', 'E', 's', 'S', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ires*/'i', 'I', 'r', 'R', 'e', 'E', 's', 'S', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*íeis*/'í', 'Í', 'e', 'E', 'i', 'I', 's', 'S', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ados*/'a', 'A', 'd', 'D', 'o', 'O', 's', 'S', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*idos*/'i', 'I', 'd', 'D', 'o', 'O', 's', 'S', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ámos*/'á', 'Á', 'm', 'M', 'o', 'O', 's', 'S', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*amos*/'a', 'A', 'm', 'M', 'o', 'O', 's', 'S', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*emos*/'e', 'E', 'm', 'M', 'o', 'O', 's', 'S', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*imos*/'i', 'I', 'm', 'M', 'o', 'O', 's', 'S', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*iras*/'i', 'I', 'r', 'R', 'a', 'A', 's', 'S', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ada*/'a', 'A', 'd', 'D', 'a', 'A', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ida*/'i', 'I', 'd', 'D', 'a', 'A', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ará*/'a', 'A', 'r', 'R', 'á', 'Á', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ara*/'a', 'A', 'r', 'R', 'a', 'A', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*erá*/'e', 'E', 'r', 'R', 'á', 'Á', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*era*/'e', 'E', 'r', 'R', 'a', 'A', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*irá*/'i', 'I', 'r', 'R', 'á', 'Á', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ava*/'a', 'A', 'v', 'V', 'a', 'A', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*iam*/'i', 'I', 'a', 'A', 'm', 'M', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ado*/'a', 'A', 'd', 'D', 'o', 'O', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ido*/'i', 'I', 'd', 'D', 'o', 'O', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ias*/'i', 'I', 'a', 'A', 's', 'S', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ais*/'a', 'A', 'i', 'I', 's', 'S', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*eis*/'e', 'E', 'i', 'I', 's', 'S', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ira*/'i', 'I', 'r', 'R', 'a', 'A', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ia*/'i', 'I', 'a', 'A', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ei*/'e', 'E', 'i', 'I', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*am*/'a', 'A', 'm', 'M', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*em*/'e', 'E', 'm', 'M', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ar*/'a', 'A', 'r', 'R', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*er*/'e', 'E', 'r', 'R', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ir*/'i', 'I', 'r', 'R', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*as*/'a', 'A', 's', 'S', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*es*/'e', 'E', 's', 'S', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*is*/'i', 'I', 's', 'S', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*eu*/'e', 'E', 'u', 'U', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*iu*/'i', 'I', 'u', 'U', false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ou*/'o', 'O', 'u', 'U', false) )
				{
				//NOOP (fall through to branching statement)
				}

			if (original_length != text.length() )
				{
				m_step1_step2_altered = true;
				step_3(text);
				}
			}
		///Delete suffix i if in RV and preceded by c 
		//---------------------------------------------
		void step_3(std::basic_string<Tchar_type, Tchar_traits>& text) 
			{
			if (is_suffix(text,/*ci*/'c', 'C', 'i', 'I') &&
				Parent::m_rv <= text.length()-1)
				{
				text.erase(text.end()-1, text.end() );
				update_r_sections(text);
				}
			//in case step 1 did not fully remove the standard suffix
			else if (m_altered_suffix_index >= 2 &&
				(is_either(text[m_altered_suffix_index-2], 'c', 'C') &&
				is_either(text[m_altered_suffix_index-1], 'i', 'I') ) )
				{
				//remove the 'i' that's inside of the string
				text.erase(m_altered_suffix_index-1, 1);
				update_r_sections(text);
				}
			}
		/**Alternatively, if neither steps 1 nor 2 altered the word, do step 4.
		If the word ends with one of the suffixes 

			-os a i o á í ó 

		in RV, delete it*/
		//---------------------------------------------
		void step_4(std::basic_string<Tchar_type, Tchar_traits>& text) 
			{
			if (delete_if_is_in_rv(text,/*os*/'o', 'O', 's', 'S') )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,'a', 'A') )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,'i', 'I') )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,'o', 'O') )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,'á', 'Á') )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,'í', 'Í') )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,'ó', 'Ó') )
				{
				return;
				}
			}
		/**Always do step 5.
		If the word ends with one of 

			-e é ê 

		in RV, delete it, and if preceded by gu (or ci) with the u (or i) in RV, delete the u (or i). 
		Or if the word ends ç remove the cedilla*/
		//---------------------------------------------
		void step_5(std::basic_string<Tchar_type, Tchar_traits>& text) 
			{
			if (delete_if_is_in_rv(text, 'e', 'E', false) )
				{
				if (Parent::m_rv <= text.length()-1 &&
					(is_suffix(text,/*gu*/'g', 'G', 'u', 'U') ||
					is_suffix(text,/*ci*/'c', 'C', 'i', 'I')) )
					{
					text.erase(text.end()-1, text.end() );
					update_r_sections(text);
					}
				return;
				}
			else if (delete_if_is_in_rv(text,/*é*/'é', 'É', false) )
				{
				if (Parent::m_rv <= text.length()-1 &&
					(is_suffix(text,/*gu*/'g', 'G', 'u', 'U') ||
					is_suffix(text,/*ci*/'c', 'C', 'i', 'I')) )
					{
					text.erase(text.end()-1, text.end() );
					update_r_sections(text);
					}
				return;
				}
			else if (delete_if_is_in_rv(text,/*ê*/'ê', 'Ê', false) )
				{
				if (Parent::m_rv <= text.length()-1 &&
					(is_suffix(text,/*gu*/'g', 'G', 'u', 'U') ||
					is_suffix(text,/*ci*/'c', 'C', 'i', 'I')) )
					{
					text.erase(text.end()-1, text.end() );
					update_r_sections(text);
					}
				return;
				}
			else if (text[text.length()-1] == 'Ç')
				{
				text[text.length()-1] = 'C';
				}
			else if (text[text.length()-1] == 'ç')
				{
				text[text.length()-1] = 'c';
				}
			}
		//internal data specific to Portuguese stemmer
		bool m_step1_step2_altered;
		size_t m_altered_suffix_index;
		};
	}

#endif //__PORTUGUESE_STEM_H__

/***************************************************************************
                          danish_stem.h  -  description
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

#ifndef __DANISH_STEM_H__
#define __DANISH_STEM_H__

#include "stemming.h"

namespace stemming
	{
	/**The Danish alphabet includes the following additional letters, 
		-æ   å   ø 
	The following letters are vowels: 
		-a   e   i   o   u   y   æ   å   ø 
	A consonant is defined as a non-vowel. 

	R2 is not used: R1 is defined in the same way as in the German stemmer.*/
	//------------------------------------------------------
	template<typename Tchar_type = char,
			typename Tchar_traits = std::char_traits<Tchar_type> >
	class danish_stem : stem<Tchar_type, Tchar_traits>
		{
	public:
		typedef stem<Tchar_type, Tchar_traits> Parent;
		//---------------------------------------------
		void operator()(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			if (text.length() < 3)
				{
				return;
				}

			//reset internal data
			Parent::m_r1 = Parent::m_r2 = Parent::m_rv =0;

			trim_western_punctuation(text);

			//see where the R1section begins
			//R1 is the first consanant after the first vowel
			find_r1(text, "aeiouyæåøAEIOUYÆÅØ");
			if (Parent::m_r1 == text.length() )
				{
				return;
				}
			///R1 must have at least 3 characters in front of it
			if (Parent::m_r1 < 3)
				{
				Parent::m_r1 = 3;
				}
			///danish does not use R2

			///step 1:
			step_1(text);
			///step 2:
			step_2(text);
			///step 3:
			step_3(text);
			///step 4:
			step_4(text);
			}
	private:
		/**Search for the longest among the following suffixes in R1, and perform the action indicated. 

			-#hed   ethed   ered   e   erede   ende   erende   ene   erne   ere   en   heden   eren   er   heder   erer   heds   es   endes   erendes   enes   ernes   eres   ens   hedens   erens   ers   ets   erets   et   eret 
				-delete

			-#s
				-delete if preceded by a valid s-ending 

		(Of course the letter of the valid s-ending is not necessarily in R1)*/
		//---------------------------------------------
		void step_1(std::basic_string<Tchar_type, Tchar_traits>& text)
			{			
			if (delete_if_is_in_r1(text,/*erendes*/'e', 'E', 'r', 'R', 'e', 'E', 'n', 'N', 'd', 'D', 'e', 'E', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*erende*/'e', 'E', 'r', 'R', 'e', 'E', 'n', 'N', 'd', 'D', 'e', 'E', false) )
				{
				return;
				}	
			else if (delete_if_is_in_r1(text,/*hedens*/'h', 'H', 'e', 'E', 'd', 'D', 'e', 'E', 'n', 'N', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*ethed*/'e', 'E', 't', 'T', 'h', 'H', 'e', 'E', 'd', 'D', false) )
				{
				return;
				}	
			else if (delete_if_is_in_r1(text,/*erede*/'e', 'E', 'r', 'R', 'e', 'E', 'd', 'D', 'e', 'E', false) )
				{
				return;
				}	
			else if (delete_if_is_in_r1(text,/*heden*/'h', 'H', 'e', 'E', 'd', 'D', 'e', 'E', 'n', 'N', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*heder*/'h', 'H', 'e', 'E', 'd', 'D', 'e', 'E', 'r', 'R', false) )
				{
				return;
				}	
			else if (delete_if_is_in_r1(text,/*endes*/'e', 'E', 'n', 'N', 'd', 'D', 'e', 'E', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*ernes*/'e', 'E', 'r', 'R', 'n', 'N', 'e', 'E', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*erens*/'e', 'E', 'r', 'R', 'e', 'E', 'n', 'N', 's', 'S', false) )
				{
				return;
				}	
			else if (delete_if_is_in_r1(text,/*erets*/'e', 'E', 'r', 'R', 'e', 'E', 't', 'T', 's', 'S', false) )
				{
				return;
				}	
			else if (delete_if_is_in_r1(text,/*eres*/'e', 'E', 'r', 'R', 'e', 'E', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*enes*/'e', 'E', 'n', 'N', 'e', 'E', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*heds*/'h', 'H', 'e', 'E', 'd', 'D', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*erer*/'e', 'E', 'r', 'R', 'e', 'E', 'r', 'R', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*eren*/'e', 'E', 'r', 'R', 'e', 'E', 'n', 'N', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*erne*/'e', 'E', 'r', 'R', 'n', 'N', 'e', 'E', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*ende*/'e', 'E', 'n', 'N', 'd', 'D', 'e', 'E', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*ered*/'e', 'E', 'r', 'R', 'e', 'E', 'd', 'D', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*eret*/'e', 'E', 'r', 'R', 'e', 'E', 't', 'T', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*hed*/'h', 'H', 'e', 'E', 'd', 'D', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*ets*/'e', 'E', 't', 'T', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*ere*/'e', 'E', 'r', 'R', 'e', 'E', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*ene*/'e', 'E', 'n', 'N', 'e', 'E', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*ens*/'e', 'E', 'n', 'N', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*ers*/'e', 'E', 'r', 'R', 's', 'S', false) )
				{
				return;
				}

			else if (delete_if_is_in_r1(text,/*et*/'e', 'E', 't', 'T', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*es*/'e', 'E', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*er*/'e', 'E', 'r', 'R', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*en*/'e', 'E', 'n', 'N', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,'e', 'E', false) )
				{
				return;
				}
			/**Define a valid s-ending as one of 
				-a b c d f g h j k l m n o p r t v y z å*/
			else if (is_suffix_in_r1(text,'s', 'S') )
				{
				if (string_util::is_one_of(text[text.length()-2], "abcdfghjklmnoprtvyzåABCDFGHJKLMNOPRTVYZÅ") )
					{
					text.erase(text.end()-1, text.end() );
					update_r_sections(text);
					}
				return;
				}
			}
		/**Search for one of the following suffixes in R1, and if found delete the last letter. 

			-gd   dt   gt   kt 

		(For example, friskt -> frisk)*/
		//---------------------------------------------
		void step_2(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			if (is_suffix_in_r1(text,/*gd*/'g', 'G', 'd', 'D') )
				{
				text.erase(text.end()-1, text.end() );
				update_r_sections(text);
				return;
				}
			else if (is_suffix_in_r1(text,/*dt*/'d', 'D', 't', 'T') )
				{
				text.erase(text.end()-1, text.end() );
				update_r_sections(text);
				return;
				}
			else if (is_suffix_in_r1(text,/*gt*/'g', 'G', 't', 'T') )
				{
				text.erase(text.end()-1, text.end() );
				update_r_sections(text);
				return;
				}
			else if (is_suffix_in_r1(text,/*kt*/'k', 'K', 't', 'T') )
				{
				text.erase(text.end()-1, text.end() );
				update_r_sections(text);
				return;
				}
			}
		/**If the word ends igst, remove the final st Search for the longest among the following suffixes in R1, and perform the action indicated. 

			-#ig   lig   elig   els 
				-delete, and then repeat step 2 

			-#løst 
				-replace with løs*/
		//---------------------------------------------
		void step_3(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			//do this check separately
			if (is_suffix(text,/*igst*/'i', 'I', 'g', 'G', 's', 'S', 't', 'T') )
				{
				text.erase(text.end()-2, text.end() );
				update_r_sections(text);
				}
			//now start looking for the longest suffix
			if (delete_if_is_in_r1(text,/*elig*/'e', 'E', 'l', 'L', 'i', 'I', 'g', 'G', false) )
				{
				step_2(text);
				return;
				}
			else if (is_suffix_in_r1(text,/*løst*/'l', 'L', 'ø', 'Ø', 's', 'S', 't', 'T') )
				{
				text.erase(text.end()-1, text.end() );
				update_r_sections(text);
				}
			else if (delete_if_is_in_r1(text,/*lig*/'l', 'L', 'i', 'I', 'g', 'G', false) )
				{
				step_2(text);
				return;
				}
			else if (delete_if_is_in_r1(text,/*els*/'e', 'E', 'l', 'L', 's', 'S', false) )
				{
				step_2(text);
				return;
				}
			else if (delete_if_is_in_r1(text,/*ig*/'i', 'I', 'g', 'G', false) )
				{
				step_2(text);
				return;
				}
			}
		/**Step 4: undouble 
		If the word ends with double consonant in R1, remove one of the consonants. 

		(For example, bestemmelse -> bestemmels (step 1) -> bestemm (step 3a) -> bestem in this step.)*/
		//---------------------------------------------
		void step_4(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			///undouble consecutive (same) consanants if either are in R1 section
			if (Parent::m_r1 <= text.length()-1 &&
				string_util::tolower_western(text[text.length()-2]) == string_util::tolower_western(text[text.length()-1]) )
				{
				if (!string_util::is_one_of(text[text.length()-2], "aeiouyæåøAEIOUYÆÅØ") )
					{
					text.erase(text.end()-1, text.end() );
					update_r_sections(text);
					}
				}
			}
		};
	}

#endif //__DANISH_STEM_H__

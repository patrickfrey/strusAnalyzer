/***************************************************************************
                          swedish_stem.h  -  description
                             -------------------
    begin                : Sat May 16 2004
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

#ifndef __SWEDISH_STEM_H__
#define __SWEDISH_STEM_H__

#include "stemming.h"

namespace stemming
	{
	/**The Swedish alphabet includes the following additional letters, 
		-ä   å   ö 
	The following letters are vowels: 
		-a   e   i   o   u   y   ä   å   ö 
	R2 is not used: R1 is defined in the same way as in the German stemmer.*/
	//------------------------------------------------------
	template<typename Tchar_type = char,
			typename Tchar_traits = std::char_traits<Tchar_type> >
	class swedish_stem : stem<Tchar_type, Tchar_traits>
		{
	public:
		typedef stem<Tchar_type,Tchar_traits> Parent;

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

			//see where the R1 section begins
			//R1 is the first consanant after the first vowel
			find_r1(text, "aeiouyåäöAEIOUYÅÄÖ");
			if (Parent::m_r1 == static_cast<unsigned int>(text.length() ) )
				{
				return;
				}

			///R1 must have at least 3 characters in front of it
			if (Parent::m_r1 < 3)
				{
				Parent::m_r1 = 3;	
				}
			///swedish does not use R2

			///step 1:
			step_1(text);
			///step 2:
			step_2(text);
			///step 3:
			step_3(text);
			}
	private:
		/**Search for the longest among the following suffixes in R1, and perform the action indicated. 
			-#a   arna   erna   heterna   orna   ad   e   ade   ande   arne   are   aste   en   anden   aren   heten   ern   ar   er   heter   or   as   arnas   ernas   ornas   es   ades   andes   ens   arens   hetens   erns   at   andet   het   ast 
				-delete 

			-#s 
				-delete if preceded by a valid s-ending 

		(Of course the letter of the valid s-ending is not necessarily in R1)*/
		//---------------------------------------------
		void step_1(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			if (delete_if_is_in_r1(text,/*heterna*/'h', 'H', 'e', 'E', 't', 'T', 'e', 'E', 'r', 'R', 'n', 'N', 'a', 'A', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*hetens*/'h', 'H', 'e', 'E', 't', 'T', 'e', 'E', 'n', 'N', 's', 'S', false) )
				{
				return;
				}	
			else if (delete_if_is_in_r1(text,/*arna*/'a', 'A', 'r', 'R', 'n', 'N', 'a', 'A', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*erna*/'e', 'E', 'r', 'R', 'n', 'N', 'a', 'A', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*orna*/'o', 'O', 'r', 'R', 'n', 'N', 'a', 'A', false) )
				{
				return;
				}	
			else if (delete_if_is_in_r1(text,/*ande*/'a', 'A', 'n', 'N', 'd', 'D', 'e', 'E', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*arne*/'a', 'A', 'r', 'R', 'n', 'N', 'e', 'E', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*aste*/'a', 'A', 's', 'S', 't', 'T', 'e', 'E', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*anden*/'a', 'A', 'n', 'N', 'd', 'D', 'e', 'E', 'n', 'N', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*heten*/'h', 'H', 'e', 'E', 't', 'T', 'e', 'E', 'n', 'N', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*heter*/'h', 'H', 'e', 'E', 't', 'T', 'e', 'E', 'r', 'R', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*arnas*/'a', 'A', 'r', 'R', 'n', 'N', 'a', 'A', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*ernas*/'e', 'E', 'r', 'R', 'n', 'N', 'a', 'A', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*ornas*/'o', 'O', 'r', 'R', 'n', 'N', 'a', 'A', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*arens*/'a', 'A', 'r', 'R', 'e', 'E', 'n', 'N', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*andet*/'a', 'A', 'n', 'N', 'd', 'D', 'e', 'E', 't', 'T', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*andes*/'a', 'A', 'n', 'N', 'd', 'D', 'e', 'E', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*aren*/'a', 'A', 'r', 'R', 'e', 'E', 'n', 'N', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text, /*erns*/'e', 'E', 'r', 'R', 'n', 'N', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*ades*/'a', 'A', 'd', 'D', 'e', 'E', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*are*/'a', 'A', 'r', 'R', 'e', 'E', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*ade*/'a', 'A', 'd', 'D', 'e', 'E', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*het*/'h', 'H', 'e', 'E', 't', 'T', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*ast*/'a', 'A', 's', 'S', 't', 'T', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text, /*ens*/'e', 'E', 'n', 'N', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*ern*/'e', 'E', 'r', 'R', 'n', 'N', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*at*/'a', 'A', 't', 'T', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*es*/'e', 'E', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*or*/'o', 'O', 'r', 'R', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*ad*/'a', 'A', 'd', 'D', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*as*/'a', 'A', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*ar*/'a', 'A', 'r', 'R', false) )
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
			else if (delete_if_is_in_r1(text, 'a', 'A', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text, 'e', 'E', false) )
				{
				return;
				}
			/**Define a valid s-ending as one of 
				-b c d f g h j k l m n o p r t v y*/
			else if (is_suffix_in_r1(text, 's', 'S') )
				{
				if (string_util::is_one_of(text[text.length()-2],
					"bcdfghjklmnoprtvyBCDFGHJKLMNOPRTVY") )
					{
					text.erase(text.end()-1, text.end() );
					update_r_sections(text);
					}
				return;
				}
			}
		/**Search for one of the following suffixes in R1, and if found delete the last letter. 

			-dd   gd   nn   dt   gt   kt   tt 

		(For example, friskt -> frisk, fröknarnn -> fröknarn)*/
		//---------------------------------------------
		void step_2(std::basic_string<Tchar_type, Tchar_traits>& text)
			{			
			if (is_suffix_in_r1(text,/*dd*/'d', 'D', 'd', 'D') )
				{
				text.erase(text.end()-1, text.end() );
				update_r_sections(text);
				}
			else if (is_suffix_in_r1(text,/*gd*/'g', 'G', 'd', 'D') )
				{
				text.erase(text.end()-1, text.end() );
				update_r_sections(text);
				}
			else if (is_suffix_in_r1(text,/*nn*/'n', 'N', 'n', 'N') )
				{
				text.erase(text.end()-1, text.end() );
				update_r_sections(text);
				}
			else if (is_suffix_in_r1(text,/*dt*/'d', 'D', 't', 'T') )
				{
				text.erase(text.end()-1, text.end() );
				update_r_sections(text);
				}
			else if (is_suffix_in_r1(text,/*gt*/'g', 'G', 't', 'T') )
				{
				text.erase(text.end()-1, text.end() );
				update_r_sections(text);
				}
			else if (is_suffix_in_r1(text,/*kt*/'k', 'K', 't', 'T') )
				{
				text.erase(text.end()-1, text.end() );
				update_r_sections(text);
				}
			else if (is_suffix_in_r1(text,/*tt*/'t', 'T', 't', 'T') )
				{
				text.erase(text.end()-1, text.end() );
				update_r_sections(text);
				}
			}
		/**Search for the longest among the following suffixes in R1, and perform the action indicated. 
			-lig   ig   els 
				-delete 

			-löst 
				-replace with lös 

			-fullt 
				-replace with full*/
		//---------------------------------------------
		void step_3(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			if (is_suffix_in_r1(text,/*fullt*/'f', 'F', 'u', 'U', 'l', 'L', 'l', 'L', 't', 'T') )
				{
				text.erase(text.end()-1, text.end() );
				update_r_sections(text);
				}
			else if (is_suffix_in_r1(text,/*löst*/'l', 'L', 'ö', 'Ö', 's', 'S', 't', 'T') )
				{
				text.erase(text.end()-1, text.end() );
				update_r_sections(text);
				}
			else if (delete_if_is_in_r1(text,/*lig*/'l', 'L', 'i', 'I', 'g', 'G', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*els*/'e', 'E', 'l', 'L', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*ig*/'i', 'I', 'g', 'G', false) )
				{
				return;
				}
			}
		};
	}

#endif //__SWEDISH_STEM_H__

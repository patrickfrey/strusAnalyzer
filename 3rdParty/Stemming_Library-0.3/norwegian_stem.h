/***************************************************************************
                          norwegian_stem.h  -  description
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

#ifndef __NORWEGIAN_STEM_H__
#define __NORWEGIAN_STEM_H__

#include "stemming.h"

namespace stemming
	{
	/**The Norwegian alphabet includes the following additional letters, 
		-æ   å   ø
	The following letters are vowels: 
		-a   e   i   o   u   y   æ   å   ø
	R2 is not used: R1 is defined in the same way as in the German stemmer.*/
	//------------------------------------------------------
	template<typename Tchar_type = char,
			typename Tchar_traits = std::char_traits<Tchar_type> >
	class norwegian_stem : stem<Tchar_type, Tchar_traits>
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
			trim_western_punctuation(text);

			//reset internal data
			Parent::m_r1 = Parent::m_r2 = Parent::m_rv =0;

			find_r1(text, "aeioøuyåæÅAÆEIOØUY");
			if (Parent::m_r1 == static_cast<unsigned int>(text.length() ) )
				{
				return;
				}
			///R1 must have at least 3 characters in front of it
			if (Parent::m_r1 < 3)
				{
				Parent::m_r1 = 3;
				}
			///norwegian does not use R2

			///step 1:
			step_1(text);
			///step 2:
			step_2(text);
			////step 3:
			step_3(text);
			}
	private:
		/**Search for the longest among the following suffixes in R1, and perform the action indicated. 
			-#a e ede ande ende ane ene hetene en heten ar er heter as es edes
			  endes enes hetenes ens hetens ers ets et het ast 
				-delete 
			-#s 
				-delete if preceded by a valid s-ending 

			-#erte   ert 
				-replace with er 

		(Of course the letter of the valid s-ending is not necessarily in R1)*/
		//---------------------------------------------
		void step_1(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			if (delete_if_is_in_r1(text,/*hetenes*/'h', 'H', 'e', 'E', 't', 'T', 'e', 'E', 'n', 'N', 'e', 'E', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*hetene*/'h', 'H', 'e', 'E', 't', 'T', 'e', 'E', 'n', 'N', 'e', 'E', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*hetens*/'h', 'H', 'e', 'E', 't', 'T', 'e', 'E', 'n', 'N', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*heter*/'h', 'H', 'e', 'E', 't', 'T', 'e', 'E', 'r', 'R', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*heten*/'h', 'H', 'e', 'E', 't', 'T', 'e', 'E', 'n', 'N', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*endes*/'e', 'E', 'n', 'N', 'd', 'D', 'e', 'E', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*ande*/'a', 'A', 'n', 'N', 'd', 'D', 'e', 'E', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*ende*/'e', 'E', 'n', 'N', 'd', 'D', 'e', 'E', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*edes*/'e', 'E', 'd', 'D', 'e', 'E', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*enes*/'e', 'E', 'n', 'N', 'e', 'E', 's', 'S', false) )
				{
				return;
				}
			if (is_suffix_in_r1(text,/*erte*/'e', 'E', 'r', 'R', 't', 'T', 'e', 'E') )
				{
				text.erase(text.end()-2, text.end() );
				update_r_sections(text);
				return;
				}
			else if (delete_if_is_in_r1(text,/*ers*/'e', 'E', 'r', 'R', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*ets*/'e', 'E', 't', 'T', 's', 'S', false) )
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
			else if (delete_if_is_in_r1(text,/*ens*/'e', 'E', 'n', 'N', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*ene*/'e', 'E', 'n', 'N', 'e', 'E', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*ane*/'a', 'A', 'n', 'N', 'e', 'E', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*ede*/'e', 'E', 'd', 'D', 'e', 'E', false) )
				{
				return;
				}
			else if (is_suffix_in_r1(text,/*ert*/'e', 'E', 'r', 'R', 't', 'T') )
				{
				text.erase(text.end()-1, text.end() );
				update_r_sections(text);
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
				-b c d f g h j k l m n o p r t v y z*/
			else if (is_suffix_in_r1(text, 's', 'S') )
				{
				if (Parent::m_r1 <= text.length()-1)
					{
					if (string_util::is_one_of(text[text.length()-2],
						"bcdfghjklmnoprtvyzBCDFGHJKLMNOPRTVYZ") )
						{
						text.erase(text.end()-1, text.end() );
						update_r_sections(text);
						return;
						}
					}
				}
			}

		/**If the word ends dt or vt in R1, delete the t.
		(For example, meldt -> meld, operativt -> operativ)*/
		//---------------------------------------------
		void step_2(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			if (is_suffix_in_r1(text,/*dt*/'d', 'D', 't', 'T') )
				{
				text.erase(text.end()-1, text.end() );
				update_r_sections(text);
				return;
				}
			else if (is_suffix_in_r1(text,/*vt*/'v', 'V', 't', 'T') )
				{
				text.erase(text.end()-1, text.end() );
				update_r_sections(text);
				return;
				}
			}
		/**Search for the longest among the following suffixes in R1, and if found, delete. 

			-leg eleg ig eig lig elig els lov elov slov hetslov*/
		//---------------------------------------------
		void step_3(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			if (delete_if_is_in_r1(text,/*hetslov*/'h', 'H', 'e', 'E', 't', 'T', 's', 'S', 'l', 'L', 'o', 'O', 'v', 'V', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*slov*/'s', 'S', 'l', 'L', 'o', 'O', 'v', 'V', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*elov*/'e', 'E', 'l', 'L', 'o', 'O', 'v', 'V', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*elig*/'e', 'E', 'l', 'L', 'i', 'I', 'g', 'G', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*eleg*/'e', 'E', 'l', 'L', 'e', 'E', 'g', 'G', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*lov*/'l', 'L', 'o', 'O', 'v', 'V', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*els*/'e', 'E', 'l', 'L', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*lig*/'l', 'L', 'i', 'I', 'g', 'G', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*eig*/'e', 'E', 'i', 'I', 'g', 'G', false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*leg*/'l', 'L', 'e', 'E', 'g', 'G', false) )
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

#endif //__NORWEGIAN_STEM_H__

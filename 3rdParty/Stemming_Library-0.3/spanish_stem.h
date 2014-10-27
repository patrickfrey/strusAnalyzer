/***************************************************************************
                          spanish_stem.h  -  description
                             -------------------
    begin                : Sat May 19 2004
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

#ifndef __SPANISH_STEM_H__
#define __SPANISH_STEM_H__

#include "stemming.h"

namespace stemming
	{
	/**Letters in Spanish include the following accented forms, 
		-á é í ó ú ü ñ 
	
	The following letters are vowels: 
		-a e i o u á é í ó ú ü 
	
	R2 is defined in the usual way - see the note on R1 and R2. 

	RV is defined as follows (and this is not the same as the French stemmer definition): 

	If the second letter is a consonant, RV is the region after the next following vowel,
	or if the first two letters are vowels, RV is the region after the next consonant,
	and otherwise (consonant-vowel case) RV is the region after the third letter.
	But RV is the end of the word if these positions cannot be found. 

	For example, 

		m a c h o     o l i v a     t r a b a j o     á u r e o
			|...|         |...|         |.......|         |...|*/
	//------------------------------------------------------
	template<typename Tchar_type = char,
			typename Tchar_traits = std::char_traits<Tchar_type> >
	class spanish_stem : stem<Tchar_type,Tchar_traits>
		{
	public:
		typedef stem<Tchar_type,Tchar_traits> Parent;

		//---------------------------------------------
		void operator()(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			if (text.length() < 3)
				{
				remove_spanish_acutes(text);
				return;
				}

			//reset internal data
			Parent::m_r1 = Parent::m_r2 = Parent::m_rv =0;

			trim_western_punctuation(text);

			find_r1(text, "aeiouáéíóúüAEIOUÁÉÍÓÚÜ");
			find_r2(text, "aeiouáéíóúüAEIOUÁÉÍÓÚÜ");
			find_spanish_rv(text, "aeiouáéíóúüAEIOUÁÉÍÓÚÜ");

			step_0(text);
			step_1(text);
			///steps 2a and 2b and only called from step1
			step_3(text);

			remove_spanish_acutes(text);
			}
	private:
		///Always do steps 0 and 1.
		/**Search for the longest among the following suffixes 

			-me se sela selo selas selos la le lo las les los nos 

			and delete it, if comes after one of 

			-#iéndo ándo ár ér ír
			-#ando iendo ar er ir
			-#yendo following u 

		in RV. In the case of (c), yendo must lie in RV, but the preceding u can be outside it. 

		In the case of (a), deletion is followed by removing the acute accent
		(for example, haciéndola -> haciendo).*/
		//---------------------------------------------
		void step_0(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			if (is_suffix_in_rv(text,/*selos*/'s', 'S', 'e', 'E', 'l', 'L', 'o', 'O', 's', 'S'))
				{
				step_0a(text, 5);
				step_0b(text, 5);
				step_0c(text, 5);
				return;
				}
			else if (is_suffix_in_rv(text,/*selas*/'s', 'S', 'e', 'E', 'l', 'L', 'a', 'A', 's', 'S'))
				{
				step_0a(text, 5);
				step_0b(text, 5);
				step_0c(text, 5);
				return;
				}
			else if (is_suffix_in_rv(text,/*sela*/'s', 'S', 'e', 'E', 'l', 'L', 'a', 'A'))
				{
				step_0a(text, 4);
				step_0b(text, 4);
				step_0c(text, 4);
				return;
				}
			else if (is_suffix_in_rv(text,/*selo*/'s', 'S', 'e', 'E', 'l', 'L', 'o', 'O'))
				{
				step_0a(text, 4);
				step_0b(text, 4);
				step_0c(text, 4);
				return;
				}
			else if (is_suffix_in_rv(text,/*las*/'l', 'L', 'a', 'A', 's', 'S'))
				{
				step_0a(text, 3);
				step_0b(text, 3);
				step_0c(text, 3);
				return;
				}
			else if (is_suffix_in_rv(text,/*les*/'l', 'L', 'e', 'E', 's', 'S'))
				{
				step_0a(text, 3);
				step_0b(text, 3);
				step_0c(text, 3);
				return;
				}
			else if (is_suffix_in_rv(text,/*los*/'l', 'L', 'o', 'O', 's', 'S'))
				{
				step_0a(text, 3);
				step_0b(text, 3);
				step_0c(text, 3);
				return;
				}
			else if (is_suffix_in_rv(text,/*nos*/'n', 'N', 'o', 'O', 's', 'S'))
				{
				step_0a(text, 3);
				step_0b(text, 3);
				step_0c(text, 3);
				return;
				}
			else if (is_suffix_in_rv(text,/*la*/'l', 'L', 'a', 'A'))
				{
				step_0a(text, 2);
				step_0b(text, 2);
				step_0c(text, 2);
				return;
				}
			else if (is_suffix_in_rv(text,/*le*/'l', 'L', 'e', 'E'))
				{
				step_0a(text, 2);
				step_0b(text, 2);
				step_0c(text, 2);
				return;
				}
			else if (is_suffix_in_rv(text,/*lo*/'l', 'L', 'o', 'O'))
				{
				step_0a(text, 2);
				step_0b(text, 2);
				step_0c(text, 2);
				return;
				}
			else if (is_suffix_in_rv(text,/*me*/'m', 'M', 'e', 'E'))
				{
				step_0a(text, 2);
				step_0b(text, 2);
				step_0c(text, 2);
				return;
				}
			else if (is_suffix_in_rv(text,/*se*/'s', 'S', 'e', 'E'))
				{
				step_0a(text, 2);
				step_0b(text, 2);
				step_0c(text, 2);
				return;
				}
			}
		///@todo should these be returning a boolean? see italian stemmer...
		//---------------------------------------------
		void step_0a(std::basic_string<Tchar_type, Tchar_traits>& text, size_t suffix_length)
			{
			if ((text.length() >= suffix_length + 5) &&
				Parent::m_rv <= static_cast<int>(text.length()-5-suffix_length) &&
				/*iéndo*/
				(is_either(text[text.length()-5-suffix_length], 'i', 'I') &&
					is_either(text[text.length()-4-suffix_length], 'é', 'É') &&
					is_either(text[text.length()-3-suffix_length], 'n', 'N') &&
					is_either(text[text.length()-2-suffix_length], 'd', 'D') &&
					is_either(text[text.length()-1-suffix_length], 'o', 'O') ) )
				{
				text.erase(text.end()-suffix_length, text.end() );
				text[text.length()-4] = 'e';
				update_r_sections(text);
				}
			else if ((text.length() >= suffix_length + 4) &&
				Parent::m_rv <= static_cast<int>(text.length()-4-suffix_length) &&
				/*ándo*/
				(is_either(text[text.length()-4-suffix_length], 'á', 'Á') &&
					is_either(text[text.length()-3-suffix_length], 'n', 'N') &&
					is_either(text[text.length()-2-suffix_length], 'd', 'D') &&
					is_either(text[text.length()-1-suffix_length], 'o', 'O') ) )
				{
				text.erase(text.end()-suffix_length, text.end() );
				text[text.length()-4] = 'a';
				update_r_sections(text);
				}
			else if ((text.length() >= suffix_length + 2) &&
				Parent::m_rv <= static_cast<int>(text.length()-2-suffix_length) &&
				/*ár*/
				(is_either(text[text.length()-2-suffix_length], 'á', 'Á') &&
					is_either(text[text.length()-1-suffix_length], 'r', 'R') ) )
				{
				text.erase(text.end()-suffix_length, text.end() );
				text[text.length()-2] = 'a';
				update_r_sections(text);
				}
			else if ((text.length() >= suffix_length + 2) &&
				Parent::m_rv <= static_cast<int>(text.length()-2-suffix_length) &&
				/*ér*/
				(is_either(text[text.length()-2-suffix_length], 'é', 'É') &&
					is_either(text[text.length()-1-suffix_length], 'r', 'R') ) )
				{
				text.erase(text.end()-suffix_length, text.end() );
				text[text.length()-2] = 'e';
				update_r_sections(text);
				}
			else if ((text.length() >= suffix_length + 2) &&
				Parent::m_rv <= static_cast<int>(text.length()-2-suffix_length) &&
				/*ír*/
				(is_either(text[text.length()-2-suffix_length], 'í', 'Í') &&
					is_either(text[text.length()-1-suffix_length], 'r', 'R') ) )
				{
				text.erase(text.end()-suffix_length, text.end() );
				text[text.length()-2] = 'i';
				update_r_sections(text);
				}
			}

		//---------------------------------------------
		void step_0b(std::basic_string<Tchar_type, Tchar_traits>& text, size_t suffix_length)
			{
			if ((text.length() >= suffix_length + 5) &&
				Parent::m_rv <= static_cast<int>(text.length()-5-suffix_length) &&
				/*iendo*/
				(is_either(text[text.length()-5-suffix_length], 'i', 'I') &&
					is_either(text[text.length()-4-suffix_length], 'e', 'E') &&
					is_either(text[text.length()-3-suffix_length], 'n', 'N') &&
					is_either(text[text.length()-2-suffix_length], 'd', 'D') &&
					is_either(text[text.length()-1-suffix_length], 'o', 'O') ) )
				{
				text.erase(text.end()-suffix_length, text.end() );
				update_r_sections(text);
				}
			else if ((text.length() >= suffix_length + 4) &&
				Parent::m_rv <= static_cast<int>(text.length()-4-suffix_length) &&
				/*ando*/
				(is_either(text[text.length()-4-suffix_length], 'a', 'A') &&
					is_either(text[text.length()-3-suffix_length], 'n', 'N') &&
					is_either(text[text.length()-2-suffix_length], 'd', 'D') &&
					is_either(text[text.length()-1-suffix_length], 'o', 'O') ) )
				{
				text.erase(text.end()-suffix_length, text.end() );
				update_r_sections(text);
				}
			else if ((text.length() >= suffix_length + 2) &&
				Parent::m_rv <= static_cast<int>(text.length()-2-suffix_length) &&
				/*ar*/
				(is_either(text[text.length()-2-suffix_length], 'a', 'A') &&
					is_either(text[text.length()-1-suffix_length], 'r', 'R') ) )
				{
				text.erase(text.end()-suffix_length, text.end() );
				update_r_sections(text);
				}
			else if ((text.length() >= suffix_length + 2) &&
				Parent::m_rv <= static_cast<int>(text.length()-2-suffix_length) &&
				/*er*/
				(is_either(text[text.length()-2-suffix_length], 'e', 'E') &&
					is_either(text[text.length()-1-suffix_length], 'r', 'R') ) )
				{
				text.erase(text.end()-suffix_length, text.end() );
				update_r_sections(text);
				}
			else if ((text.length() >= suffix_length + 2) &&
				Parent::m_rv <= static_cast<int>(text.length()-2-suffix_length) &&
				/*ir*/
				(is_either(text[text.length()-2-suffix_length], 'i', 'I') &&
					is_either(text[text.length()-1-suffix_length], 'r', 'R') ) )
				{
				text.erase(text.end()-suffix_length, text.end() );
				update_r_sections(text);
				}
			}

		//---------------------------------------------
		void step_0c(std::basic_string<Tchar_type, Tchar_traits>& text, size_t suffix_length)
			{
			if ((text.length() >= suffix_length + 6) &&
				Parent::m_rv <= text.length()-(suffix_length+5) &&
				/*uyendo*/
				(is_either(text[text.length()-6-suffix_length], 'u', 'U') &&
					is_either(text[text.length()-5-suffix_length], 'y', 'Y') &&
					is_either(text[text.length()-4-suffix_length], 'e', 'E') &&
					is_either(text[text.length()-3-suffix_length], 'n', 'N') &&
					is_either(text[text.length()-2-suffix_length], 'd', 'D') &&
					is_either(text[text.length()-1-suffix_length], 'o', 'O') ) )
				{
				text.erase(text.end()-suffix_length, text.end() );
				update_r_sections(text);
				}
			}
		/**Search for the longest among the following suffixes, and perform the action indicated. 

			-anza anzas ico ica icos icas ismo ismos able ables ible ibles ista istas oso osa osos osas amiento amientos imiento imientos 
				-delete if in R2 

			-adora ador ación adoras adores aciones 
				-delete if in R2 
				-if preceded by ic, delete if in R2 

			-logía logías 
				-replace with log if in R2 

			-ución uciones 
				-replace with u if in R2 

			-encia encias 
				-replace with ente if in R2 

			-amente 
				-delete if in R1 
				-if preceded by iv, delete if in R2 (and if further preceded by at, delete if in R2), otherwise, 
				-if preceded by os, ic or ad, delete if in R2 

			-mente 
				-delete if in R2 
				-if preceded by able or ible, delete if in R2 

			-idad idades 
				-delete if in R2 
				-if preceded by abil, ic or iv, delete if in R2 

			-iva ivo ivas ivos 
				-delete if in R2 
				-if preceded by at, delete if in R2*/
		//---------------------------------------------
		void step_1(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			size_t original_length = text.length();
			if (delete_if_is_in_r2(text,/*imientos*/'i', 'I', 'm', 'M', 'i', 'I', 'e', 'E', 'n', 'N', 't', 'T', 'o', 'O', 's', 'S') )
				{
				if (original_length > text.length() )
					{
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*amientos*/'a', 'A', 'm', 'M', 'i', 'I', 'e', 'E', 'n', 'N', 't', 'T', 'o', 'O', 's', 'S') )
				{
				if (original_length > text.length() )
					{
					return;
					}
				step_2a(text);
				}
			else if (is_suffix_in_r2(text,/*uciones*/'u', 'U', 'c', 'C', 'i', 'I', 'o', 'O', 'n', 'N', 'e', 'E', 's', 'S') )
				{
				text.erase(text.end()-6, text.end() );
				update_r_sections(text);
				return;
				}
			else if (delete_if_is_in_r2(text,/*amiento*/'a', 'A', 'm', 'M', 'i', 'I', 'e', 'E', 'n', 'N', 't', 'T', 'o', 'O') )
				{
				if (original_length > text.length() )
					{
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*imiento*/'i', 'I', 'm', 'M', 'i', 'I', 'e', 'E', 'n', 'N', 't', 'T', 'o', 'O') )
				{
				if (original_length > text.length() )
					{
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*aciones*/'a', 'A', 'c', 'C', 'i', 'I', 'o', 'O', 'n', 'N', 'e', 'E', 's', 'S') )
				{
				if (original_length > text.length() )
					{
					delete_if_is_in_r2(text,/*ic*/'i', 'I', 'c', 'C');
					return;
					}
				step_2a(text);
				}
			else if (is_suffix_in_r2(text,/*logías*/'l', 'L', 'o', 'O', 'g', 'G', 'í', 'Í', 'a', 'A', 's', 'S') )
				{
				text.erase(text.end()-3, text.end() );
				update_r_sections(text);
				return;
				}
			else if (is_suffix_in_r2(text,/*encias*/'e', 'E', 'n', 'N', 'c', 'C', 'i', 'I', 'a', 'A', 's', 'S') )
				{
				text.erase(text.end()-2, text.end() );
				text[text.length()-2] = 't';
				text[text.length()-1] = 'e';
				update_r_sections(text);
				return;
				}
			else if (delete_if_is_in_r2(text,/*idades*/'i', 'I', 'd', 'D', 'a', 'A', 'd', 'D', 'e', 'E', 's', 'S') )
				{
				if (original_length > text.length() )
					{
					if (delete_if_is_in_r2(text,/*abil*/'a', 'A', 'b', 'B', 'i', 'I', 'l', 'L') ||
						delete_if_is_in_r2(text,/*ic*/'i', 'I', 'c', 'C') ||
						delete_if_is_in_r2(text,/*iv*/'i', 'I', 'v', 'V') )
						{
						return;
						}
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r1(text,/*amente*/'a', 'A', 'm', 'M', 'e', 'E', 'n', 'N', 't', 'T', 'e', 'E') )
				{
				if (original_length > text.length() )
					{
					if (delete_if_is_in_r2(text,/*iv*/'i', 'I', 'v', 'V') )
						{
						delete_if_is_in_r2(text,/*at*/'a', 'A', 't', 'T');
						return;
						}
					else
						{
						if (delete_if_is_in_r2(text,/*os*/'o', 'O', 's', 'S') ||
							delete_if_is_in_r2(text,/*ic*/'i', 'I', 'c', 'C') ||
							delete_if_is_in_r2(text,/*ad*/'a', 'A', 'd', 'D') )
							{
							return;
							}
						}
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*adores*/'a', 'A', 'd', 'D', 'o', 'O', 'r', 'R', 'e', 'E', 's', 'S') )
				{
				if (original_length > text.length() )
					{
					delete_if_is_in_r2(text,/*ic*/'i', 'I', 'c', 'C');
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*adoras*/'a', 'A', 'd', 'D', 'o', 'O', 'r', 'R', 'a', 'A', 's', 'S') )
				{
				if (original_length > text.length() )
					{
					delete_if_is_in_r2(text,/*ic*/'i', 'I', 'c', 'C');
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*adora*/'a', 'A', 'd', 'D', 'o', 'O', 'r', 'R', 'a', 'A') )
				{
				if (original_length > text.length() )
					{
					delete_if_is_in_r2(text,/*ic*/'i', 'I', 'c', 'C');
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*ación*/'a', 'A', 'c', 'C', 'i', 'I', 'ó', 'Ó', 'n', 'N') )
				{
				if (original_length > text.length() )
					{
					delete_if_is_in_r2(text,/*ic*/'i', 'I', 'c', 'C');
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*ibles*/'i', 'I', 'b', 'B', 'l', 'L', 'e', 'E', 's', 'S') )
				{
				if (original_length > text.length() )
					{
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*istas*/'i', 'I', 's', 'S', 't', 'T', 'a', 'A', 's', 'S') )
				{
				if (original_length > text.length() )
					{
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*ables*/'a', 'A', 'b', 'B', 'l', 'L', 'e', 'E', 's', 'S') )
				{
				if (original_length > text.length() )
					{
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*ismos*/'i', 'I', 's', 'S', 'm', 'M', 'o', 'O', 's', 'S') )
				{
				if (original_length > text.length() )
					{
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*anzas*/'a', 'A', 'n', 'N', 'z', 'Z', 'a', 'A', 's', 'S') )
				{
				if (original_length > text.length() )
					{
					return;
					}
				step_2a(text);
				}
			else if (is_suffix_in_r2(text,/*logía*/'l', 'L', 'o', 'O', 'g', 'G', 'í', 'Í', 'a', 'A') )
				{
				text.erase(text.end()-2, text.end() );
				update_r_sections(text);
				return;
				}
			else if (is_suffix_in_r2(text,/*ución*/'u', 'U', 'c', 'C', 'i', 'I', 'ó', 'Ó', 'n', 'N') )
				{
				text.erase(text.end()-4, text.end() );
				update_r_sections(text);
				return;
				}
			else if (is_suffix_in_r2(text,/*encia*/'e', 'E', 'n', 'N', 'c', 'C', 'i', 'I', 'a', 'A') )
				{
				text.erase(text.end()-1, text.end() );
				text[text.length()-2] = 't';
				text[text.length()-1] = 'e';
				update_r_sections(text);
				return;
				}
			else if (delete_if_is_in_r2(text,/*mente*/'m', 'M', 'e', 'E', 'n', 'N', 't', 'T', 'e', 'E') )
				{
				if (original_length > text.length() )
					{
					if (delete_if_is_in_r2(text,/*able*/'a', 'A', 'b', 'B', 'l', 'L', 'e', 'E') ||
						delete_if_is_in_r2(text,/*ible*/'i', 'I', 'b', 'B', 'l', 'L', 'e', 'E') )
						{
						return;
						}
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*anza*/'a', 'A', 'n', 'N', 'z', 'Z', 'a', 'A') )
				{
				if (original_length > text.length() )
					{
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*icos*/'i', 'I', 'c', 'C', 'o', 'O', 's', 'S') )
				{
				if (original_length > text.length() )
					{
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*icas*/'i', 'I', 'c', 'C', 'a', 'A', 's', 'S') )
				{
				if (original_length > text.length() )
					{
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*ismo*/'i', 'I', 's', 'S', 'm', 'M', 'o', 'O') )
				{
				if (original_length > text.length() )
					{
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*able*/'a', 'A', 'b', 'B', 'l', 'L', 'e', 'E') )
				{
				if (original_length > text.length() )
					{
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*ible*/'i', 'I', 'b', 'B', 'l', 'L', 'e', 'E') )
				{
				if (original_length > text.length() )
					{
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*ista*/'i', 'I', 's', 'S', 't', 'T', 'a', 'A') )
				{
				if (original_length > text.length() )
					{
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*osos*/'o', 'O', 's', 'S', 'o', 'O', 's', 'S') )
				{
				if (original_length > text.length() )
					{
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*osas*/'o', 'O', 's', 'S', 'a', 'A', 's', 'S') )
				{
				if (original_length > text.length() )
					{
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*ivas*/'i', 'I', 'v', 'V', 'a', 'A', 's', 'S') )
				{
				if (original_length > text.length() )
					{
					delete_if_is_in_r2(text,/*at*/'a', 'A', 't', 'T');
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*ivos*/'i', 'I', 'v', 'V', 'o', 'O', 's', 'S') )
				{
				if (original_length > text.length() )
					{
					delete_if_is_in_r2(text,/*at*/'a', 'A', 't', 'T');
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*ador*/'a', 'A', 'd', 'D', 'o', 'O', 'r', 'R') )
				{
				if (original_length > text.length() )
					{
					delete_if_is_in_r2(text,/*ic*/'i', 'I', 'c', 'C');
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*idad*/'i', 'I', 'd', 'D', 'a', 'A', 'd', 'D') )
				{
				if (original_length > text.length() )
					{
					if (delete_if_is_in_r2(text,/*abil*/'a', 'A', 'b', 'B', 'i', 'I', 'l', 'L') ||
						delete_if_is_in_r2(text,/*ic*/'i', 'I', 'c', 'C') ||
						delete_if_is_in_r2(text,/*iv*/'i', 'I', 'v', 'V') )
						{
						return;
						}
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*ico*/'i', 'I', 'c', 'C', 'o', 'O') )
				{
				if (original_length > text.length() )
					{
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*ica*/'i', 'I', 'c', 'C', 'a', 'A') )
				{
				if (original_length > text.length() )
					{
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*oso*/'o', 'O', 's', 'S', 'o', 'O') )
				{
				if (original_length > text.length() )
					{
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*osa*/'o', 'O', 's', 'S', 'a', 'A') )
				{
				if (original_length > text.length() )
					{
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*iva*/'i', 'I', 'v', 'V', 'a', 'A') )
				{
				if (original_length > text.length() )
					{
					delete_if_is_in_r2(text,/*at*/'a', 'A', 't', 'T');
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*ivo*/'i', 'I', 'v', 'V', 'o', 'O') )
				{
				if (original_length > text.length() )
					{
					delete_if_is_in_r2(text,/*at*/'a', 'A', 't', 'T');
					return;
					}
				step_2a(text);
				}	
			//this should only be called from here if nothing was removed in step 1
			if (text.length() == original_length)
				{
				step_2a(text);
				}
			}
		///Do step 2a if no ending was removed by step 1.
		/**Search for the longest among the following suffixes in RV, and if found, delete if preceded by u. 

			-ya ye yan yen yeron yendo yo yó yas yes yais yamos 

		(Note that the preceding u need not be in RV.)*/
		//---------------------------------------------
		void step_2a(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			size_t original_length = text.length();
			if (is_suffix_in_rv(text,/*yeron*/'y', 'Y', 'e', 'E', 'r', 'R', 'o', 'O', 'n', 'N'))
				{
				if (is_either(text[text.length()-6], 'u', 'U') )
					{
					text.erase(text.end()-5, text.end() );
					update_r_sections(text);
					return;
					}
				step_2b(text);
				}
			else if (is_suffix_in_rv(text,/*yendo*/'y', 'Y', 'e', 'E', 'n', 'N', 'd', 'D', 'o', 'O'))
				{
				if (is_either(text[text.length()-6], 'u', 'U') )
						{
					text.erase(text.end()-5, text.end() );
					update_r_sections(text);
					return;
					}
				step_2b(text);
				}
			else if (is_suffix_in_rv(text,/*yamos*/'y', 'Y', 'a', 'A', 'm', 'M', 'o', 'O', 's', 'S'))
				{
				if (is_either(text[text.length()-6], 'u', 'U') )
					{
					text.erase(text.end()-5, text.end() );
					update_r_sections(text);
					return;
					}
				step_2b(text);
				}
			else if (is_suffix_in_rv(text,/*yais*/'y', 'Y', 'a', 'A', 'i', 'I', 's', 'S'))
				{
				if (is_either(text[text.length()-5], 'u', 'U') )
					{
					text.erase(text.end()-4, text.end() );
					update_r_sections(text);
					return;
					}
				step_2b(text);
				}
			else if (is_suffix_in_rv(text,/*yan*/'y', 'Y', 'a', 'A', 'n', 'N'))
				{
				if (is_either(text[text.length()-4], 'u', 'U') )
					{
					text.erase(text.end()-3, text.end() );
					update_r_sections(text);
					return;
					}
				step_2b(text);
				}
			else if (is_suffix_in_rv(text,/*yen*/'y', 'Y', 'e', 'E', 'n', 'N'))
				{
				if (is_either(text[text.length()-4], 'u', 'U') )
					{
					text.erase(text.end()-3, text.end() );
					update_r_sections(text);
					return;
					}
				step_2b(text);
				}
			else if (is_suffix_in_rv(text,/*yas*/'y', 'Y', 'a', 'A', 's', 'S'))
				{
				if (is_either(text[text.length()-4], 'u', 'U') )
					{
					text.erase(text.end()-3, text.end() );
					update_r_sections(text);
					return;
					}
				step_2b(text);
				}
			else if (is_suffix_in_rv(text,/*yes*/'y', 'Y', 'e', 'E', 's', 'S'))
				{
				if (is_either(text[text.length()-4], 'u', 'U') )
					{
					text.erase(text.end()-3, text.end() );
					update_r_sections(text);
					return;
					}
				step_2b(text);
				}
			else if (is_suffix_in_rv(text,/*ya*/'y', 'Y', 'a', 'A'))
				{
				if (is_either(text[text.length()-3], 'u', 'U') )
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					return;
					}
				step_2b(text);
				}
			else if (is_suffix_in_rv(text,/*ye*/'y', 'Y', 'e', 'E'))
				{
				if (is_either(text[text.length()-3],'u', 'U') )
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					return;
					}
				step_2b(text);
				}
			else if (is_suffix_in_rv(text,/*yo*/'y', 'Y', 'o', 'O'))
				{
				if (is_either(text[text.length()-3], 'u', 'U') )
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					return;
					}
				step_2b(text);
				}
			else if (is_suffix_in_rv(text,/*yó*/'y', 'Y', 'ó', 'Ó'))
				{
				if (is_either(text[text.length()-3], 'u', 'U') )
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					return;
					}
				step_2b(text);
				}
			//only called if 2a fails to remove a suffix
			if (text.length() == original_length)
				{
				step_2b(text);
				}
			}
		///Do Step 2b if step 2a was done, but failed to remove a suffix.
		/**Search for the longest among the following suffixes in RV, and perform the action indicated. 

			-en es éis emos 
				-delete, and if preceded by gu delete the u (the gu need not be in RV) 

			-arían arías arán arás aríais aría aréis aríamos aremos ará aré erían erías
			erán erás eríais ería eréis eríamos eremos erá eré irían irías irán irás iríais
			iría iréis iríamos iremos irá iré aba ada ida ía ara iera ad ed id ase iese aste
			iste an aban ían aran ieran asen iesen aron ieron ado ido ando iendo ió ar er ir
			as abas adas idas ías aras ieras ases ieses ís áis abais íais arais ierais
			aseis ieseis asteis isteis ados idos amos ábamos íamos imos áramos iéramos iésemos ásemos 
				-delete*/
		//---------------------------------------------
		void step_2b(std::basic_string<Tchar_type, Tchar_traits>& text) 
			{
			if (delete_if_is_in_rv(text,/*aríamos*/'a', 'A', 'r', 'R', 'í', 'Í', 'a', 'A', 'm', 'M', 'o', 'O', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*eríamos*/'e', 'E', 'r', 'R', 'í', 'Í', 'a', 'A', 'm', 'M', 'o', 'O', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*iríamos*/'i', 'I', 'r', 'R', 'í', 'Í', 'a', 'A', 'm', 'M', 'o', 'O', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*iéramos*/'i', 'I', 'é', 'É', 'r', 'R', 'a', 'A', 'm', 'M', 'o', 'O', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*iésemos*/'i', 'I', 'é', 'É', 's', 'S', 'e', 'E', 'm', 'M', 'o', 'O', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*aríais*/'a', 'A', 'r', 'R', 'í', 'Í', 'a', 'A', 'i', 'I', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*aremos*/'a', 'A', 'r', 'R', 'e', 'E', 'm', 'M', 'o', 'O', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*eríais*/'e', 'E', 'r', 'R', 'í', 'Í', 'a', 'A', 'i', 'I', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*eremos*/'e', 'E', 'r', 'R', 'e', 'E', 'm', 'M', 'o', 'O', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*iríais*/'i', 'I', 'r', 'R', 'í', 'Í', 'a', 'A', 'i', 'I', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*iremos*/'i', 'I', 'r', 'R', 'e', 'E', 'm', 'M', 'o', 'O', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ierais*/'i', 'I', 'e', 'E', 'r', 'R', 'a', 'A', 'i', 'I', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ieseis*/'i', 'I', 'e', 'E', 's', 'S', 'e', 'E', 'i', 'I', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*asteis*/'a', 'A', 's', 'S', 't', 'T', 'e', 'E', 'i', 'I', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*isteis*/'i', 'I', 's', 'S', 't', 'T', 'e', 'E', 'i', 'I', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ábamos*/'á', 'Á', 'b', 'B', 'a', 'A', 'm', 'M', 'o', 'O', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*áramos*/'á', 'Á', 'r', 'R', 'a', 'A', 'm', 'M', 'o', 'O', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ásemos*/'á', 'Á', 's', 'S', 'e', 'E', 'm', 'M', 'o', 'O', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*arían*/'a', 'A', 'r', 'R', 'í', 'Í', 'a', 'A', 'n', 'N', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*arías*/'a', 'A', 'r', 'R', 'í', 'Í', 'a', 'A', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*aréis*/'a', 'A', 'r', 'R', 'é', 'É', 'i', 'I', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*erían*/'e', 'E', 'r', 'R', 'í', 'Í', 'a', 'A', 'n', 'N', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*erías*/'e', 'E', 'r', 'R', 'í', 'Í', 'a', 'A', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*eréis*/'e', 'E', 'r', 'R', 'é', 'É', 'i', 'I', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*irían*/'i', 'I', 'r', 'R', 'í', 'Í', 'a', 'A', 'n', 'N', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*irías*/'i', 'I', 'r', 'R', 'í', 'Í', 'a', 'A', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*iréis*/'i', 'I', 'r', 'R', 'é', 'É', 'i', 'I', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ieran*/'i', 'I', 'e', 'E', 'r', 'R', 'a', 'A', 'n', 'N', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*iesen*/'i', 'I', 'e', 'E', 's', 'S', 'e', 'E', 'n', 'N', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ieron*/'i', 'I', 'e', 'E', 'r', 'R', 'o', 'O', 'n', 'N', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*iendo*/'i', 'I', 'e', 'E', 'n', 'N', 'd', 'D', 'o', 'O', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ieras*/'i', 'I', 'e', 'E', 'r', 'R', 'a', 'A', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ieses*/'i', 'I', 'e', 'E', 's', 'S', 'e', 'E', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*abais*/'a', 'A', 'b', 'B', 'a', 'A', 'i', 'I', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*arais*/'a', 'A', 'r', 'R', 'a', 'A', 'i', 'I', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*aseis*/'a', 'A', 's', 'S', 'e', 'E', 'i', 'I', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*íamos*/'í', 'Í', 'a', 'A', 'm', 'M', 'o', 'O', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*emos*/'e', 'E', 'm', 'M', 'o', 'O', 's', 'S', false) )
				{
				if (is_suffix(text,/*gu*/'g', 'G', 'u', 'U') )
					{
					text.erase(text.end()-1, text.end() );
					update_r_sections(text);
					}
				return;
				}
			else if (delete_if_is_in_rv(text,/*arán*/'a', 'A', 'r', 'R', 'á', 'Á', 'n', 'N', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*arás*/'a', 'A', 'r', 'R', 'á', 'Á', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*aría*/'a', 'A', 'r', 'R', 'í', 'Í', 'a', 'A', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*erán*/'e', 'E', 'r', 'R', 'á', 'Á', 'n', 'N', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*erás*/'e', 'E', 'r', 'R', 'á', 'Á', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ería*/'e', 'E', 'r', 'R', 'í', 'Í', 'a', 'A', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*irán*/'i', 'I', 'r', 'R', 'á', 'Á', 'n', 'N', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*irás*/'i', 'I', 'r', 'R', 'á', 'Á', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*iría*/'i', 'I', 'r', 'R', 'í', 'Í', 'a', 'A', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*iera*/'i', 'I', 'e', 'E', 'r', 'R', 'a', 'A', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*iese*/'i', 'I', 'e', 'E', 's', 'S', 'e', 'E', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*aste*/'a', 'A', 's', 'S', 't', 'T', 'e', 'E', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*iste*/'i', 'I', 's', 'S', 't', 'T', 'e', 'E', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*aban*/'a', 'A', 'b', 'B', 'a', 'A', 'n', 'N', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*aran*/'a', 'A', 'r', 'R', 'a', 'A', 'n', 'N', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*asen*/'a', 'A', 's', 'S', 'e', 'E', 'n', 'N', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*aron*/'a', 'A', 'r', 'R', 'o', 'O', 'n', 'N', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ando*/'a', 'A', 'n', 'N', 'd', 'D', 'o', 'O', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*abas*/'a', 'A', 'b', 'B', 'a', 'A', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*adas*/'a', 'A', 'd', 'D', 'a', 'A', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*idas*/'i', 'I', 'd', 'D', 'a', 'A', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*aras*/'a', 'A', 'r', 'R', 'a', 'A', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ases*/'a', 'A', 's', 'S', 'e', 'E', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*íais*/'í', 'Í', 'a', 'A', 'i', 'I', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ados*/'a', 'A', 'd', 'D', 'o', 'O', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*idos*/'i', 'I', 'd', 'D', 'o', 'O', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*amos*/'a', 'A', 'm', 'M', 'o', 'O', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*imos*/'i', 'I', 'm', 'M', 'o', 'O', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ará*/'a', 'A', 'r', 'R', 'á', 'Á', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*aré*/'a', 'A', 'r', 'R', 'é', 'É', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*erá*/'e', 'E', 'r', 'R', 'á', 'Á', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*eré*/'e', 'E', 'r', 'R', 'é', 'É', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*irá*/'i', 'I', 'r', 'R', 'á', 'Á', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*iré*/'i', 'I', 'r', 'R', 'é', 'É', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*aba*/'a', 'A', 'b', 'B', 'a', 'A', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ada*/'a', 'A', 'd', 'D', 'a', 'A', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ida*/'i', 'I', 'd', 'D', 'a', 'A', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ara*/'a', 'A', 'r', 'R', 'a', 'A', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ase*/'a', 'A', 's', 'S', 'e', 'E', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ían*/'í', 'Í', 'a', 'A', 'n', 'N', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ado*/'a', 'A', 'd', 'D', 'o', 'O', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ido*/'i', 'I', 'd', 'D', 'o', 'O', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ías*/'í', 'Í', 'a', 'A', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*áis*/'á', 'Á', 'i', 'I', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*éis*/'é', 'É', 'i', 'I', 's', 'S', false) )
				{
				if (is_suffix(text,/*gu*/'g', 'G', 'u', 'U') )
					{
					text.erase(text.end()-1, text.end() );
					update_r_sections(text);
					}
				return;
				}
			else if (delete_if_is_in_rv(text,/*ía*/'í', 'Í', 'a', 'A', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ad*/'a', 'A', 'd', 'D', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ed*/'e', 'E', 'd', 'D', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*id*/'i', 'I', 'd', 'D', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*an*/'a', 'A', 'n', 'N', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ió*/'i', 'I', 'ó', 'Ó', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ar*/'a', 'A', 'r', 'R', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*er*/'e', 'E', 'r', 'R', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ir*/'i', 'I', 'r', 'R', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*as*/'a', 'A', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ís*/'í', 'Í', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*en*/'e', 'E', 'n', 'N', false) )
				{
				if (is_suffix(text,/*gu*/'g', 'G', 'u', 'U') )
					{
					text.erase(text.end()-1, text.end() );
					update_r_sections(text);
					}
				return;
				}
			else if (delete_if_is_in_rv(text,/*es*/'e', 'E', 's', 'S', false) )
				{
				if (is_suffix(text,/*gu*/'g', 'G', 'u', 'U') )
					{
					text.erase(text.end()-1, text.end() );
					update_r_sections(text);
					}
				return;
				}
			}
		///Always do step 3.
		/**Search for the longest among the following suffixes in RV, and perform the action indicated. 

			-os a o á í ó 
				-delete if in RV 

			-e é 
				-delete if in RV, and if preceded by gu with the u in RV delete the u*/
		//---------------------------------------------
		void step_3(std::basic_string<Tchar_type, Tchar_traits>& text) 
			{
			if (delete_if_is_in_rv(text,/*os*/'o', 'O', 's', 'S') )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*a*/'a', 'A') )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*o*/'o', 'O') )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*á*/'á', 'Á') )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*í*/'í', 'Í') )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ó*/'ó', 'Ó') )
				{
				return;
				}

			else if (delete_if_is_in_rv(text,/*é*/'é', 'É') ||
					delete_if_is_in_rv(text,/*e*/'e', 'E') )
				{
				if (is_suffix_in_rv(text,/*u*/'u', 'U'))
					{
					if (is_either(text[text.length()-2], 'g', 'G') )
						{
						text.erase(text.end()-1, text.end() );
						}
					}
				return;
				}
			}
		};
	}

#endif //__SPANISH_STEM_H__

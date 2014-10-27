/***************************************************************************
                          finnish_stem.h  -  description
                             -------------------
    begin                : Sat May 11 2004
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

#ifndef __FINNISH_STEM_H__
#define __FINNISH_STEM_H__

#include "stemming.h"

namespace stemming
	{
	/**Finnish is not an Indo-European language, but belongs to the Finno-Ugric group,
	which again belongs to the Uralic group. Distinctions between a-, i- and d-suffixes
	can be made in Finnish, but they are much less sharply separated than in an
	Indo-European language. The system of endings is extremely elaborate, but strictly
	defined, and applies equally to all nominals, that is, to nouns, adjectives and pronouns.
	Verb endings have a close similarity to nominal endings, which again makes Finnish
	very different from any Indo-European language. 

	More problematical than the endings themselves is the change that can be effected in a
	stem as a result of taking a particular ending. A stem typically has two forms
	,strong and weak, where one class of ending follows the strong form and the complementary
	class the weak. Normalising strong and weak forms after ending removal is not generally possible,
	although the common case where strong and weak forms only differ in the single or double form of
	a final consonant can be dealt with. 

	Finnish includes the following accented forms, 
		-ä   ö 
	The following letters are vowels: 
		-a   e   i   o   u   y   ä   ö

	R1 and R2 are then defined in the usual way.*/
	//------------------------------------------------------
	template<typename Tchar_type = char,
			typename Tchar_traits = std::char_traits<Tchar_type> >
	class finnish_stem : stem<Tchar_type, Tchar_traits>
		{
	public:
		typedef stem<Tchar_type,Tchar_traits> Parent;

		finnish_stem() : m_step_3_successful(false) {}
		//---------------------------------------------
		void operator()(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			if (text.length() < 2)
				{
				return;
				}

			//reset internal data
			m_step_3_successful = false;
			Parent::m_r1 = Parent::m_r2 = Parent::m_rv =0;

			trim_western_punctuation(text);

			find_r1(text, "aeiouyäöAEIOUYÄÖ");
			find_r2(text, "aeiouyäöAEIOUYÄÖ");

			step_1(text);
			step_2(text);
			step_3(text);
			step_4(text);
			step_5(text);
			step_6(text);
			}
	private:
		/**Step 1: particles etc 
		Search for the longest among the following suffixes in R1, and perform the action indicated 

			-# kin   kaan   kään   ko   kö   han   hän   pa   pä
				- delete if preceded by n, t or a vowel
			-# sti
				- delete if in R2
			(Of course, the n, t or vowel of 1(a) need not be in R1:
			only the suffix removed must be in R1. And similarly below*/
		//---------------------------------------------
		void step_1(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			if (is_suffix_in_r1(text,/*kaan*/'k', 'K', 'a', 'A', 'a', 'A', 'n', 'N') ||
				is_suffix_in_r1(text,/*kään*/'k', 'K', 'ä', 'Ä', 'ä', 'Ä', 'n', 'N') )
				{
				if (text.length() >= 5 &&
					string_util::is_one_of(text[text.length()-5], "ntaeiouyäöNTAEIOUYÄÖ") )
					{
					text.erase(text.end()-4, text.end() );
					update_r_sections(text);
					}
				return;
				}
			else if (is_suffix_in_r1(text,/*kin*/'k', 'K', 'i', 'I', 'n', 'N') ||
				is_suffix_in_r1(text,/*han*/'h', 'H', 'a', 'A', 'n', 'N') ||
				is_suffix_in_r1(text,/*hän*/'h', 'H', 'ä', 'Ä', 'n', 'N') )
				{
				if (text.length() >= 4 &&
					string_util::is_one_of(text[text.length()-4], "ntaeiouyäöNTAEIOUYÄÖ") )
					{
					text.erase(text.end()-3, text.end() );
					update_r_sections(text);
					}
				return;
				}
			else if (is_suffix_in_r1(text,/*sti*/'s', 'S', 't', 'T', 'i', 'I') )
				{
				delete_if_is_in_r2(text,/*sti*/'s', 'S', 't', 'T', 'i', 'I');
				return;
				}
			else if (is_suffix_in_r1(text,/*ko*/'k', 'K', 'o', 'O') ||
				is_suffix_in_r1(text,/*kö*/'k', 'K', 'ö', 'Ö') ||
				is_suffix_in_r1(text,/*pa*/'p', 'P', 'a', 'A') ||
				is_suffix_in_r1(text,/*pä*/'p', 'P', 'ä', 'Ä') )
				{
				if (text.length() >= 3 &&
					string_util::is_one_of(text[text.length()-3], "ntaeiouyäöNTAEIOUYÄÖ") )
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					}
				return;
				}
			}
		/**Step 2: possessives 
		Search for the longest among the following suffixes in R1, and perform the action indicated 

			- si 
				- delete if not preceded by k 
			- ni 
				- delete 
				- if preceded by kse, replace with ksi 
			- nsa   nsä   mme   nne 
				- delete 
			- an 
				- delete if preceded by one of   ta   ssa   sta   lla   lta   na 
			- än 
				- delete if preceded by one of   tä   ssä   stä   llä   ltä   nä 
			- en 
				- delete if preceded by one of   lle   ine*/
		//---------------------------------------------
		void step_2(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			if (delete_if_is_in_r1(text,/*nsa*/'n', 'N', 's', 'S', 'a', 'A', false) ||
				delete_if_is_in_r1(text,/*nsä*/'n', 'N', 's', 'S', 'ä', 'Ä', false) ||
				delete_if_is_in_r1(text,/*mme*/'m', 'M', 'm', 'M', 'e', 'E', false) ||
				delete_if_is_in_r1(text,/*nne*/'n', 'N', 'n', 'N', 'e', 'E', false) )
				{
				return;
				}
			else if (is_suffix_in_r1(text,/*si*/'s', 'S', 'i', 'I') )
				{
				if (text.length() >= 3 &&
					!(text[text.length()-3] == 'k' || text[text.length()-3] == 'K'))
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					}
				return;
				}
			else if (delete_if_is_in_r1(text,/*ni*/'n', 'N', 'i', 'I', false) )
				{
				if (is_suffix(text, /*kse*/'k','K','s','S','e','E') )
					{
					text[text.length()-1] = 'i';
					}
				return;
				}
			else if (is_suffix_in_r1(text,/*an*/'a', 'A', 'n', 'N') )
				{
				if ((text.length() >= 4 &&
					(text.compare(text.length()-4, 2, "ta", 2) == 0 ||
					text.compare(text.length()-4, 2, "na", 2) == 0) ) ||
					(text.length() >= 5 &&
					(text.compare(text.length()-5, 3, "ssa", 3) == 0 ||
					text.compare(text.length()-5, 3, "sta", 3) == 0 ||
					text.compare(text.length()-5, 3, "lla", 3) == 0 ||
					text.compare(text.length()-5, 3, "lta", 3) == 0) ) )
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					}
				return;
				}
			else if (is_suffix_in_r1(text,/*än*/'ä', 'Ä', 'n', 'N') )
				{
				if ((text.length() >= 4 &&
					(text.compare(text.length()-4, 2, "tä", 2) == 0 ||
					text.compare(text.length()-4, 2, "nä", 2) == 0) ) ||
					(text.length() >= 5 &&
					(text.compare(text.length()-5, 3, "ssä", 3) == 0 ||
					text.compare(text.length()-5, 3, "stä", 3) == 0 ||
					text.compare(text.length()-5, 3, "llä", 3) == 0 ||
					text.compare(text.length()-5, 3, "ltä", 3) == 0) ) )
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					}
				return;
				}
			else if (is_suffix_in_r1(text,/*en*/'e', 'E', 'n', 'N') )
				{
				if (text.length() >= 5 &&
					(text.compare(text.length()-5, 3, "lle", 3) == 0 ||
					text.compare(text.length()-5, 3, "ine", 3) == 0 ) )
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					}
				return;
				}
			}

		//////////////////////////////////////////////////////////////////////////
		// Define a v (vowel) as one of   a   e   i   o   u   y   ä   ö.        //
		// Define a V (restricted vowel) as one of   a   e   i   o   u   ä   ö. //
		// So Vi means a V followed by letter i.                                //
		// Define LV (long vowel) as one of   aa   ee   ii   oo   uu   ää   öö. // 
		// Define a c (consonant) as a character other than a v.                //
		// So cv means a c followed by a v.                                     //
		//////////////////////////////////////////////////////////////////////////

		/**Step 3: cases 
		Search for the longest among the following suffixes in R1, and perform the action indicated 

			- hXn   preceded by X, where X is a V other than u (a/han, e/hen etc) 
			- siin   den   tten   preceded by Vi
			- seen   preceded by LV
			- a   ä   preceded by cv
			- tta   ttä   preceded by e
			- ta   tä   ssa   ssä   sta   stä   lla   llä   lta   ltä   lle   na   nä   ksi   ine 
				- delete 
			- n
				- delete, and if preceded by LV or ie, delete the last vowel*/
		//---------------------------------------------
		void step_3(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			//seen followed by LV
			if (is_suffix_in_r1(text,/*seen*/'s', 'S', 'e', 'E', 'e', 'E', 'n', 'N') &&
				string_util::is_one_of(text[text.length()-5], "aeiouäöAEIOUÄÖ") &&
				string_util::tolower_western(text[text.length()-5]) == string_util::tolower_western(text[text.length()-6]) )
				{
				text.erase(text.end()-4, text.end() );
				update_r_sections(text);
				m_step_3_successful = true;
				return;
				}
			//suffix followed by Vi
			else if (is_either(text[text.length()-5], 'i', 'I') &&
					(is_suffix_in_r1(text,/*siin*/'s', 'S', 'i', 'I', 'i', 'I', 'n', 'N') ||
					is_suffix_in_r1(text,/*tten*/'t', 'T', 't', 'T', 'e', 'E', 'n', 'N') ) &&
					string_util::is_one_of(text[text.length()-6], "aeiouäöAEIOUÄÖ") )
				{
				text.erase(text.end()-4, text.end() );
				update_r_sections(text);
				m_step_3_successful = true;
				return;
				}
			//suffix followed by Vi
			else if (is_either(text[text.length()-4], 'i', 'I') &&
					is_suffix_in_r1(text,/*den*/'d', 'D', 'e', 'E', 'n', 'N') &&
					string_util::is_one_of(text[text.length()-5], "aeiouäöAEIOUÄÖ") )
				{
				text.erase(text.end()-3, text.end() );
				update_r_sections(text);
				m_step_3_successful = true;
				return;
				}
			else if ((is_suffix_in_r1(text,/*tta*/'t', 'T', 't', 'T', 'a', 'A') ||
				is_suffix_in_r1(text,/*ttä*/'t', 'T', 't', 'T', 'ä', 'Ä')) &&
				is_either(text[text.length()-4], 'e', 'E') )
				{
				text.erase(text.end()-3, text.end() );
				update_r_sections(text);
				m_step_3_successful = true;
				return;
				}
			//ends if VHVN
			else if (
				(is_suffix_in_r1(text,/*han*/'h', 'H', 'a', 'A', 'n', 'N') ||
				is_suffix_in_r1(text,/*hen*/'h', 'H', 'e', 'E', 'n', 'N') ||
				is_suffix_in_r1(text,/*hin*/'h', 'H', 'i', 'I', 'n', 'N') ||
				is_suffix_in_r1(text,/*hon*/'h', 'H', 'o', 'O', 'n', 'N') ||
				is_suffix_in_r1(text,/*hän*/'h', 'H', 'ä', 'Ä', 'n', 'N') ||
				is_suffix_in_r1(text,/*hön*/'h', 'H', 'ö', 'Ö', 'n', 'N') ) )
				{
				if (string_util::tolower_western(text[text.length()-2]) == string_util::tolower_western(text[text.length()-4]) )
					{
					text.erase(text.end()-3, text.end() );
					update_r_sections(text);
					m_step_3_successful = true;
					}
				return;
				}
			else if (delete_if_is_in_r1(text,/*ssa*/'s', 'S', 's', 'S', 'a', 'A', false) ||
				delete_if_is_in_r1(text,/*ssä*/'s', 'S', 's', 'S', 'ä', 'Ä', false) ||
				delete_if_is_in_r1(text,/*sta*/'s', 'S', 't', 'T', 'a', 'A', false) ||
				delete_if_is_in_r1(text,/*stä*/'s', 'S', 't', 'T', 'ä', 'Ä', false) ||
				delete_if_is_in_r1(text,/*lla*/'l', 'L', 'l', 'L', 'a', 'A', false) ||
				delete_if_is_in_r1(text,/*llä*/'l', 'L', 'l', 'L', 'ä', 'Ä', false) ||
				delete_if_is_in_r1(text,/*lta*/'l', 'L', 't', 'T', 'a', 'A', false) ||
				delete_if_is_in_r1(text,/*ltä*/'l', 'L', 't', 'T', 'ä', 'Ä', false) ||
				delete_if_is_in_r1(text,/*lle*/'l', 'L', 'l', 'L', 'e', 'E', false) ||
				delete_if_is_in_r1(text,/*ksi*/'k', 'K', 's', 'S', 'i', 'I', false) ||
				delete_if_is_in_r1(text,/*ine*/'i', 'I', 'n', 'N', 'e', 'E', false) ||
				delete_if_is_in_r1(text,/*na*/'n', 'N', 'a', 'A', false) ||
				delete_if_is_in_r1(text,/*nä*/'n', 'N', 'ä', 'Ä', false) )
				{
				m_step_3_successful = true;
				return;
				}
			else if (delete_if_is_in_r1(text,/*ta*/'t', 'T', 'a', 'A', false) ||
				delete_if_is_in_r1(text,/*tä*/'t', 'T', 'ä', 'Ä', false) )
				{
				m_step_3_successful = true;
				return;
				}
			//suffix followed by cv
			else if ((is_suffix_in_r1(text, 'a', 'A') || is_suffix_in_r1(text, 'ä', 'Ä') )&&
					!string_util::is_one_of(text[text.length()-3], "aeiouyäöAEIOUYÄÖ") &&
					string_util::is_one_of(text[text.length()-2], "aeiouyäöAEIOUYÄÖ") )
				{
				text.erase(text.end()-1, text.end() );
				update_r_sections(text);
				m_step_3_successful = true;
				return;
				}
			//suffix followed by LV or ie
			else if (is_suffix_in_r1(text, 'n', 'N') )
				{
				text.erase(text.end()-1, text.end() );
				update_r_sections(text);
				if ((string_util::is_one_of(text[text.length()-1], "aeiouäöAEIOUÄÖ") &&
					string_util::tolower_western(text[text.length()-1]) == string_util::tolower_western(text[text.length()-2])) ||
					is_suffix_in_r1(text,/*ie*/'i', 'I', 'e', 'E') ) 
					{
					text.erase(text.end()-1, text.end() );
					update_r_sections(text);
					}
				m_step_3_successful = true;
				return;
				}
			}
		/**Step 4: other endings 
		Search for the longest among the following suffixes in R2, and perform the action indicated 

			- mpi   mpa   mpä   mmi   mma   mmä 
				- delete if not preceded by po 
			- impi   impa   impä   immi   imma   immä   eja   ejä 
				- delete*/
		//---------------------------------------------
		void step_4(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			if (delete_if_is_in_r2(text,/*impi*/'i', 'I', 'm', 'M', 'p', 'P', 'i', 'I', false) ||
				delete_if_is_in_r2(text,/*impa*/'i', 'I', 'm', 'M', 'p', 'P', 'a', 'A', false) ||
				delete_if_is_in_r2(text,/*impä*/'i', 'I', 'm', 'M', 'p', 'P', 'ä', 'Ä', false) ||
				delete_if_is_in_r2(text,/*immi*/'i', 'I', 'm', 'M', 'm', 'M', 'i', 'I', false) ||
				delete_if_is_in_r2(text,/*imma*/'i', 'I', 'm', 'M', 'm', 'M', 'a', 'A', false) ||
				delete_if_is_in_r2(text,/*immä*/'i', 'I', 'm', 'M', 'm', 'M', 'ä', 'Ä', false) ||
				delete_if_is_in_r2(text,/*eja*/'e', 'E', 'j', 'J', 'a', 'A', false) ||
				delete_if_is_in_r2(text,/*ejä*/'e', 'E', 'j', 'J', 'ä', 'Ä', false) )
				{
				return;
				}
			else if (text.length() >= 5 &&
					(is_suffix_in_r2(text,/*mpi*/'m', 'M', 'p', 'P', 'i', 'I') ||
					is_suffix_in_r2(text,/*mpa*/'m', 'M', 'p', 'P', 'a', 'A') ||
					is_suffix_in_r2(text,/*mpä*/'m', 'M', 'p', 'P', 'ä', 'Ä') ||
					is_suffix_in_r2(text,/*mmi*/'m', 'M', 'm', 'M', 'i', 'I') ||
					is_suffix_in_r2(text,/*mma*/'m', 'M', 'm', 'M', 'a', 'A') ||
					is_suffix_in_r2(text,/*mmä*/'m', 'M', 'm', 'M', 'ä', 'Ä') ) )
				{
				if (!(is_either(text[text.length()-5], 'p', 'P') &&
					is_either(text[text.length()-4], 'o', 'O') ) )
					{
					text.erase(text.end()-3, text.end() );
					update_r_sections(text);
					}
				}
			return;
			}
		/**Step 5: plurals 
		If an ending was removed in step 3, delete a final i or j if in R1;
		otherwise, if an ending was not removed in step 3,
		then delete the final t in R1 if it follows a vowel, and, if a t is removed,
		then delete a final mma or imma in R2, unless the mma is preceded by po.*/
		//---------------------------------------------
		void step_5(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			//if step 3 was successful in removing a suffix
			if (m_step_3_successful)
				{
				if (delete_if_is_in_r1(text, 'i', 'I') ||
					delete_if_is_in_r1(text, 'j', 'J') )
					{
					//NOOP
					}
				}
			else
				{
				if (string_util::is_one_of(text[text.length()-2], "aeiouyäöAEIOUYÄÖ") )
					{
					if (delete_if_is_in_r1(text, 't', 'T') )
						{
						if (!delete_if_is_in_r2(text,/*imma*/'i', 'I','m', 'M', 'm', 'M', 'a', 'A') )
							{
							if (is_suffix_in_r2(text,/*mma*/'m', 'M', 'm', 'M', 'a', 'A') &&
								//isn't proceeded by "po"
								!(is_either(text[text.length()-5], 'p', 'P') &&
								is_either(text[text.length()-4], 'o', 'O') ) )
								{
								text.erase(text.end()-3, text.end() );
								update_r_sections(text);
								}
							}
						}
					}
				}
			}
		/**Step 6: tidying up 
		Perform turn steps (a), (b), (c), (d), restricting all tests to the R1 region.*/
		//---------------------------------------------
		void step_6(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			step_6a(text);
			step_6b(text);
			step_6c(text);
			step_6d(text);
			step_6e(text);
			}
		///Step a) If R1 ends with LV then delete the last letter
		//---------------------------------------------
		void step_6a(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			if (Parent::m_r1 <= text.length()-2 &&
				string_util::is_one_of(text[text.length()-1], "aeiouäöAEIOUÄÖ") &&
				string_util::tolower_western(text[text.length()-1]) == string_util::tolower_western(text[text.length()-2]))
				{
				text.erase(text.end()-1);
				update_r_sections(text);
				}
			}
		///Step b) If R1 ends with cX, c a consonant and X one of   a   ä   e   i, then delete the last letter
		//---------------------------------------------
		void step_6b(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			if (Parent::m_r1 <= text.length()-2 &&
				!string_util::is_one_of(text[text.length()-2], "aeiouyäöAEIOUYÄÖ") &&
				string_util::is_one_of(text[text.length()-1], "aeiäAEIÄ") )
				{
				text.erase(text.end()-1);
				update_r_sections(text);
				}
			}
		///Step c) If R1 ends with oj or uj then delete the last letter
		//---------------------------------------------
		void step_6c(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			if (is_suffix_in_r1(text,/*oj*/'o', 'O', 'j', 'J') ||
				is_suffix_in_r1(text,/*uj*/'u', 'U', 'j', 'J') )
				{
				text.erase(text.end()-1);
				update_r_sections(text);
				}
			}
		///Step d) If R1 ends with jo then delete the last letter 
		//---------------------------------------------
		void step_6d(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			if (is_suffix_in_r1(text,/*jo*/'j', 'J', 'o', 'O') )
				{
				text.erase(text.end()-1);
				update_r_sections(text);
				}
			}
		/**Do step (e), which is not restricted to R1.
		Step e) If the word ends with a double consonant followed by zero or more vowels,
		then remove the last consonant (so eläkk -> eläk, aatonaatto -> aatonaato)*/
		//---------------------------------------------
		void step_6e(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			//find the last consanant
			size_t index = text.find_last_not_of("aeiouyäöAEIOUYÄÖ");
			if (index == std::basic_string<Tchar_type, Tchar_traits>::npos ||
				index < 1)
				{
				return;
				}
			if (string_util::tolower_western(text[index]) == string_util::tolower_western(text[index-1]))
				{
				text.erase(text.begin()+(index) );
				update_r_sections(text);
				}
			}
		//internal data specific to Finnish stemmer
		bool m_step_3_successful;
		};
	}

#endif //__FINNISH_STEM_H__

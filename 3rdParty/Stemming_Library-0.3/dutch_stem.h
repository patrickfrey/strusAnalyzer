/***************************************************************************
                          dutch_stem.h  -  description
                             -------------------
    begin               : Sat May 11 2004
    copyright           : (C) 2004 by Blake Madden
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/

#ifndef __DUTCH_STEM_H__
#define __DUTCH_STEM_H__

#include "stemming.h"

namespace stemming
	{
	/**Dutch includes the following accented forms 
		-ä   ë   ï   ö   ü   á   é   í   ó   ú   è*/
	//------------------------------------------------------
	template<typename Tchar_type = char,
			typename Tchar_traits = std::char_traits<Tchar_type> >
	class dutch_stem : stem<Tchar_type, Tchar_traits>
		{
	public:
		typedef stem<Tchar_type, Tchar_traits> Parent;

		dutch_stem() : m_step_2_succeeded(false) {}
		//---------------------------------------------
		void operator()(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			///First, remove all umlaut and acute accents
			remove_dutch_umlauts(text);
			remove_dutch_acutes(text);

			if (text.length() < 3)
				{
				return;
				}

			//reset internal data
			m_step_2_succeeded = false;
			Parent::m_r1 = Parent::m_r2 = Parent::m_rv =0;

			trim_western_punctuation(text);

			///Hash initial y, y after a vowel, and i between vowels
			hash_dutch_yi(text, "aeiouyèAEIOUYÈ");

			find_r1(text, "aeiouyèAEIOUYÈ");
			find_r2(text, "aeiouyèAEIOUYÈ");
			///R1 must have at least 3 characters in front of it
			if (Parent::m_r1 < 3)
				{
				Parent::m_r1 = 3;
				}

			///step 1:
			step_1(text);
			///step 2:
			step_2(text);
			///step 3:
			step_3a(text);
			step_3b(text);
			///step 4:
			step_4(text);

			///unhash I and Y back into their original form 
			unhash_dutch_yi(text);
			}
	private:
		/**Step 1:
		Search for the longest among the following suffixes, and perform the action indicated: 

			-# heden 
				- replace with heid if in R1

			-# en   ene 
				- delete if in R1 and preceded by a valid en-ending, and then undouble the ending

			-# s   se 
				- delete if in R1 and preceded by a valid s-ending*/ 
		//---------------------------------------------
		void step_1(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			if (is_suffix(text,/*heden*/'h', 'H', 'e', 'E', 'd', 'D', 'e', 'E', 'n', 'N') )
				{
				if (Parent::m_r1 <= text.length()-5)
					{
					text.erase(text.end()-1, text.end() );
					update_r_sections(text);
					text[text.length()-2] = 'i';
					text[text.length()-1] = 'd';
					}
				}
			///Define a valid en-ending as a non-vowel, and not gem.
			else if (is_suffix(text,/*ene*/'e', 'E', 'n', 'N', 'e', 'E')	)
				{
				if (Parent::m_r1 <= text.length()-3 &&
					!string_util::is_one_of(text[text.length()-4], "aeiouyèAEIOUYÈ") &&
					(text.length() < 6 ||
					//"gem" in front of "en" ending
					!(is_either(text[text.length()-6], 'g', 'G') &&
					is_either(text[text.length()-5], 'e', 'E') &&
					is_either(text[text.length()-4], 'm', 'M') ) ) )
					{
					text.erase(text.end()-3, text.end() );
					//undouble dd, kk, tt
					if (string_util::is_one_of(text[text.length()-1], "kdtKDT") &&
						string_util::tolower_western(text[text.length()-2]) == string_util::tolower_western(text[text.length()-1]))
						{
						text.erase(text.end()-1, text.end() );
						}
					update_r_sections(text);
					}
				return;
				}
			else if (is_suffix(text,/*en*/'e', 'E', 'n', 'N') )
				{
				if (Parent::m_r1 <= text.length()-2 &&
					!string_util::is_one_of(text[text.length()-3], "aeiouyèAEIOUYÈ") &&
					(text.length() < 5 ||
					!(is_either(text[text.length()-5], 'g', 'G') &&
					is_either(text[text.length()-4], 'e', 'E') &&
					is_either(text[text.length()-3], 'm', 'M') ) ) )
					{
					text.erase(text.end()-2, text.end() );
					//undouble dd, kk, tt
					if (string_util::is_one_of(text[text.length()-1], "kdtKDT") &&
						string_util::tolower_western(text[text.length()-2]) == string_util::tolower_western(text[text.length()-1]) )
						{
						text.erase(text.end()-1, text.end() );
						}
					update_r_sections(text);
					}
				return;
				}
			else if (is_suffix(text,/*se*/'s', 'S', 'e', 'E') &&
				!string_util::is_one_of(text[text.length()-3], "aeèiouyjAEÈIOUYJ"))
				{
				if (Parent::m_r1 <= text.length()-2)
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					return;
					}
				}
			///Define a valid s-ending as a non-vowel other than j
			else if (is_suffix(text, 's', 'S') &&
				!string_util::is_one_of(text[text.length()-2], "aeèiouyjAEÈIOUYJ"))
				{
				if (Parent::m_r1 <= text.length()-1)
					{
					text.erase(text.end()-1, text.end() );
					update_r_sections(text);
					return;
					}
				}
			}

		/**Step 2:
		Delete suffix e if in R1 and preceded by a non-vowel, and then undouble the ending*/
		//---------------------------------------------
		void step_2(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			if (is_suffix(text, 'e', 'E') )
				{
				if (Parent::m_r1 <= text.length()-1 &&
					!string_util::is_one_of(text[text.length()-2], "aeiouyèAEIOUYUÈ") )
					{
					//watch out for vowel I/Y vowel
					if (string_util::is_one_of(text[text.length()-2], "iyIY") &&
						string_util::is_one_of(text[text.length()-3], "aeiouyèAEIOUYÈ") )
						{
						return;
						}
					else
						{
						text.erase(text.end()-1, text.end() );
						//undouble dd, kk, tt
						if (string_util::is_one_of(text[text.length()-1], "kdtKDT") &&
							string_util::tolower_western(text[text.length()-2]) == string_util::tolower_western(text[text.length()-1]))
							{
							text.erase(text.end()-1, text.end() );
							}
						update_r_sections(text);
						m_step_2_succeeded = true;
						}
					}
				}
			}
		/**Step 3a: heid 
		delete heid if in R2 and not preceded by c, and treat a preceding en as in step 1(b).*/
		//---------------------------------------------
		void step_3a(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			if (is_suffix(text,/*heid*/'h', 'H', 'e', 'E', 'i', 'I', 'd', 'D') )
				{
				if (text.length() >= 5 &&
					Parent::m_r2 <= text.length()-4 &&
					is_neither(text[text.length()-5], 'c', 'C') )
					{
					text.erase(text.end()-4, text.end() );
					update_r_sections(text);
					if (Parent::m_r1 <= text.length()-2 &&
						is_suffix(text, 'e', 'E', 'n', 'N') )
						{
						if ((!string_util::is_one_of(text[text.length()-3], "aeiouyèAEIOUYÈ") ||
							(string_util::is_one_of(text[text.length()-3], "iyYI") &&
							string_util::is_one_of(text[text.length()-4], "aeiouyèAEIOUYÈ"))) &&
							(text.length() < 5 ||
							!(is_either(text[text.length()-5], 'g', 'G') &&
							is_either(text[text.length()-4], 'e', 'E') &&
							is_either(text[text.length()-3], 'm', 'M') ) ) )
							{
							text.erase(text.end()-2, text.end() );
							//undouble dd, kk, tt
							if (string_util::is_one_of(text[text.length()-1], "kdtKDT") &&
								string_util::tolower_western(text[text.length()-2]) == string_util::tolower_western(text[text.length()-1]))
								{
								text.erase(text.end()-1, text.end() );
								}
							update_r_sections(text);
							}
						}
					return;
					}
				}
			}
		/**Step 3b: d-suffixes (*) 
		Search for the longest among the following suffixes, and perform the action indicated. 

			- end   ing
				- delete if in R2.
				- if preceded by ig, delete if in R2 and not preceded by e, otherwise undouble the ending.

			- ig
				- delete if in R2 and not preceded by e.

			- lijk
				- delete if in R2, and then repeat step 2.

			- baar
				- delete if in R2.

			- bar
				- delete if in R2 and if step 2 actually removed an e.*/
		//---------------------------------------------
		void step_3b(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			if (delete_if_is_in_r2(text,/*end*/'e', 'E', 'n', 'N', 'd', 'D') ||
				delete_if_is_in_r2(text,/*ing*/'i', 'I', 'n', 'N', 'g', 'G') )
				{
				update_r_sections(text);
				if (text.length() > 3)
					{
					if (is_neither(text[text.length()-3], 'e' , 'E') &&
						delete_if_is_in_r2(text,/*ig*/'i', 'I', 'g', 'G') )
						{
						update_r_sections(text);
						return;
						}
					else
						{
						if (string_util::is_one_of(text[text.length()-1], "kdtKDT") &&
							string_util::tolower_western(text[text.length()-2]) == string_util::tolower_western(text[text.length()-1]))
							{
							text.erase(text.end()-1, text.end() );
							update_r_sections(text);
							return;
							}
						}
					}
				return;
				}
			else if (!(text.length() >= 3 && is_either(text[text.length()-3], 'e', 'E') ) &&
					delete_if_is_in_r2(text,/*ig*/'i', 'I', 'g', 'G') )
				{
				update_r_sections(text);
				return;
				}
			else if (delete_if_is_in_r2(text,/*baar*/'b', 'B', 'a', 'A', 'a', 'A', 'r', 'R') )
				{
				return;
				}
			else if (delete_if_is_in_r2(text,/*lijk*/'l', 'L', 'i', 'I', 'j', 'J', 'k', 'K') )
				{
				step_2(text);
				return;
				}
			else if (m_step_2_succeeded &&
				delete_if_is_in_r2(text,/*bar*/'b', 'B', 'a', 'A', 'r', 'R') )
				{
				return;
				}
			}
		/**Step 4: undouble vowel 
			If the words ends CVD, where C is a non-vowel, D is a non-vowel other than I,
			and V is double a, e, o or u, remove one of the vowels from V
			(for example, maan -> man, brood -> brod).*/
		//------------------------------------------------------
		void step_4(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			//undouble consecutive (same) consanants
			if (text.length() >= 4 &&
				!string_util::is_one_of(text[text.length()-4], "aeiouyèAEIOUYÈ") &&
				!string_util::is_one_of(text[text.length()-1], "aeiouyèAEIOUYÈ") &&
				text[text.length()-1] != LOWER_I_HASH &&
				text[text.length()-1] != UPPER_I_HASH &&
				string_util::is_one_of(text[text.length()-2], "aeouAEOU") &&
				string_util::tolower_western(text[text.length()-2]) == string_util::tolower_western(text[text.length()-3]) )
				{
				text.erase(text.end()-2, text.end()-1);
				update_r_sections(text);
				}
			}
		//internal data specific to Dutch stemmer
		bool m_step_2_succeeded;
		};
	}

#endif //__DUTCH_STEM_H__

/***************************************************************************
                          french_stem.h  -  description
                             -------------------
    begin                : Sat May 25 2004
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

#ifndef __FRENCH_STEM_H__
#define __FRENCH_STEM_H__

#include "stemming.h"

namespace stemming
	{
	/**Letters in French include the following accented forms:

		-â à ç ë é ê è ï î ô û ù 

	The following letters are vowels:
	
		-a e i o u y â à ë é ê è ï î ô û ù*/
	//------------------------------------------------------
	template<typename Tchar_type = char,
			typename Tchar_traits = std::char_traits<Tchar_type> >
	class french_stem : stem<Tchar_type, Tchar_traits>
		{
	public:
		typedef stem<Tchar_type,Tchar_traits> Parent;

		french_stem() : m_step_1_successful(false) {}
		//---------------------------------------------
		void operator()(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			if (text.length() < 2)
				{
				return;
				}

			//reset internal data
			m_step_1_successful = false;
			Parent::m_r1 = Parent::m_r2 = Parent::m_rv =0;

			trim_western_punctuation(text);
			hash_french_yui(text, "aeiouyâàëéêèïîôûùAEIOUYÂÀËÉÊÈÏÎÔÛÙ");

			find_r1(text, "aeiouyâàëéêèïîôûùAEIOUYÂÀËÉÊÈÏÎÔÛÙ");
			find_r2(text, "aeiouyâàëéêèïîôûùAEIOUYÂÀËÉÊÈÏÎÔÛÙ");
			find_french_rv(text, "aeiouyâàëéêèïîôûùAEIOUYÂÀËÉÊÈÏÎÔÛÙ");

			size_t length = text.length();
			step_1(text);
			if (!m_step_1_successful)
				{
				step_2a(text);
				}
			if (length != text.length() )
				{
				step_3(text);
				}
			else
				{
				step_4(text);
				}
			step_5(text);
			step_6(text);

			unhash_french_yui(text);
			}
	private:
		bool ic_to_iqu(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			if (is_suffix(text,/*ic*/'i', 'I', 'c', 'C') )
				{
				if (Parent::m_r2 <= text.length()-2)
					{
					text.erase(text.length()-2);
					update_r_sections(text);
					return true;
					}
				else
					{
					text.erase(text.length()-2);
					///todo make this more case sensitive
					text += "iq";
					text += LOWER_U_HASH;
					update_r_sections(text);
					return true;
					}
				}
			return false;
			}
		/**Search for the longest among the following suffixes, and perform the action indicated. 

			-ance iqUe isme able iste eux ances iqUes ismes ables istes 
				-delete if in R2 

			-atrice ateur ation atrices ateurs ations 
				-delete if in R2 
				-if preceded by ic, delete if in R2, else replace by iqU 

			-logie logies 
				-replace with log if in R2 

			-usion ution usions utions 
				-replace with u if in R2 

			-ence ences 
				-replace with ent if in R2 

			-ement ements 
				-delete if in RV 
				-if preceded by iv, delete if in R2 (and if further preceded by at, delete if in R2), otherwise, 
				-if preceded by eus, delete if in R2, else replace by eux if in R1, otherwise, 
				-if preceded by abl or iqU, delete if in R2, otherwise, 
				-if preceded by ièr or Ièr, replace by i if in RV 

			-ité ités 
				-delete if in R2 
				-if preceded by abil, delete if in R2, else replace by abl, otherwise, 
				-if preceded by ic, delete if in R2, else replace by iqU, otherwise, 
				-if preceded by iv, delete if in R2 

			-if ive ifs ives 
				-delete if in R2 
				-if preceded by at, delete if in R2 (and if further preceded by ic, delete if in R2, else replace by iqU) 

			-eaux 
				-replace with eau 

			-aux 
				-replace with al if in R1 

			-euse euses 
				-delete if in R2, else replace by eux if in R1 

			-issement issements 
				-delete if in R1 and preceded by a non-vowel 

			-amment 
				-replace with ant if in RV 

			-emment 
				-replace with ent if in RV 

			-ment ments 
				-delete if preceded by a vowel in RV*/
		//---------------------------------------------
		void step_1(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			size_t length = text.length();
			if (is_suffix(text,/*issements*/'i', 'I', 's', 'S', 's', 'S', 'e', 'E', 'm', 'M', 'e', 'E', 'n', 'N', 't', 'T', 's', 'S') )
				{
				if (Parent::m_r1 <= static_cast<unsigned int>(text.length()-9) &&
					!string_util::is_one_of(text[text.length()-10], "aeiouyâàëéêèïîôûùAEIOUYÂÀËÉÊÈÏÎÔÛÙ") )
					{
					text.erase(text.end()-9, text.end() );
					m_step_1_successful = true;
					}
				return;
				}
			else if (is_suffix(text,/*issement*/'i', 'I', 's', 'S', 's', 'S', 'e', 'E', 'm', 'M', 'e', 'E', 'n', 'N', 't', 'T') )
				{
				if (Parent::m_r1 <= static_cast<unsigned int>(text.length()-8) &&
					!string_util::is_one_of(text[text.length()-9], "aeiouyâàëéêèïîôûùAEIOUYÂÀËÉÊÈÏÎÔÛÙ") )
					{
					text.erase(text.end()-8, text.end() );
					m_step_1_successful = true;
					}
				return;
				}
			//7
			else if (delete_if_is_in_r2(text,/*atrices*/'a', 'A', 't', 'T', 'r', 'R', 'i', 'I', 'c', 'C', 'e', 'E', 's', 'S', false) )
				{
				if (length != text.length() )
					{
					ic_to_iqu(text);
					m_step_1_successful = true;
					}
				return;
				}
			//6
			else if (is_suffix(text,/*amment*/'a', 'A', 'm', 'M', 'm', 'M', 'e', 'E', 'n', 'N', 't', 'T') )
				{
				if (Parent::m_rv <= static_cast<unsigned int>(text.length()-6) )
					{
					///todo make this more case sensitive
					text.replace(text.end()-5, text.end(), "nt");
					update_r_sections(text);
					}
				return;
				}
			else if (is_suffix(text,/*emment*/'e', 'E', 'm', 'M', 'm', 'M', 'e', 'E', 'n', 'N', 't', 'T') )
				{
				if (Parent::m_rv <= static_cast<unsigned int>(text.length()-6) )
					{
					///todo make this more case sensitive
					text.replace(text.end()-5, text.end(), "nt");
					update_r_sections(text);
					}
				return;
				}
			else if (is_suffix(text,/*logies*/'l', 'L', 'o', 'O', 'g', 'G', 'i', 'I', 'e', 'E', 's', 'S') )
				{
				if (Parent::m_r2 <= static_cast<unsigned int>(text.length()-6) )
					{
					text.erase(text.end()-3, text.end() );
					update_r_sections(text);
					m_step_1_successful = true;
					}
				return;
				}
			else if (delete_if_is_in_r2(text,/*atrice*/'a', 'A', 't', 'T', 'r', 'R', 'i', 'I', 'c', 'C', 'e', 'E', false) ||
					delete_if_is_in_r2(text,/*ateurs*/'a', 'A', 't', 'T', 'e', 'E', 'u', 'U', 'r', 'R', 's', 'S', false) ||
					delete_if_is_in_r2(text,/*ations*/'a', 'A', 't', 'T', 'i', 'I', 'o', 'O', 'n', 'N', 's', 'S', false) )
				{				
				if (length != text.length() )
					{
					ic_to_iqu(text);
					m_step_1_successful = true;
					}
				return;
				}
			else if (is_suffix(text,/*usions*/'u', 'U', 's', 'S', 'i', 'I', 'o', 'O', 'n', 'N', 's', 'S') ||
					is_suffix(text,/*utions*/'u', 'U', 't', 'T', 'i', 'I', 'o', 'O', 'n', 'N', 's', 'S') )
				{
				if (Parent::m_r2 <= static_cast<unsigned int>(text.length()-6) )
					{
					text.erase(text.end()-5, text.end() );
					update_r_sections(text);
					m_step_1_successful = true;
					}
				return;
				}
			else if (delete_if_is_in_rv(text,/*ements*/'e', 'E', 'm', 'M', 'e', 'E', 'n', 'N', 't', 'T', 's', 'S', false) )
				{
				if (delete_if_is_in_r2(text,/*iv*/'i', 'I', 'v', 'V') )
					{
					delete_if_is_in_r2(text,/*at*/'a', 'A', 't', 'T');
					}
				else if (is_suffix(text,/*eus*/'e', 'E', 'u', 'U', 's', 'S') )
					{
					if (Parent::m_r2 <= text.length()-3)
						{
						text.erase(text.length()-3);
						update_r_sections(text);
						}
					else if (Parent::m_r1 <= text.length()-3)
						{
						///todo better case sensitivity here
						text[text.length()-1] = 'x';
						}
					}
				else if (delete_if_is_in_r2(text,/*abl*/'a', 'A', 'b', 'B', 'l', 'L') )
					{
					//NOOP
					}
				else if (text.length() >= 3 &&
					(text[text.length()-3] == 'i' || text[text.length()-3] == 'I') &&
					(text[text.length()-2] == 'q' || text[text.length()-2] == 'Q') &&
					is_either(text[text.length()-1], LOWER_U_HASH, UPPER_U_HASH) )
					{
					if (Parent::m_r2 <= text.length()-3)
						{
						text.erase(text.end()-3, text.end() );
						update_r_sections(text);
						}
					}
				else if (is_suffix_in_r2(text,/*ièr*/'i', 'I', 'è', 'È', 'r', 'R') )
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					}
				else if (Parent::m_r2 <= static_cast<int>(text.length()-3) &&
					(text[text.length()-2] == 'è' || text[text.length()-2] == 'È') &&
					(text[text.length()-1] == 'r' || text[text.length()-1] == 'R') &&
					is_either(text[text.length()-3], LOWER_I_HASH, UPPER_I_HASH) )
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					}
				if (length != text.length() )
					{
					m_step_1_successful = true;
					}
				return;
				}
			//5
			else if (delete_if_is_in_r2(text,/*ateur*/'a', 'A', 't', 'T', 'e', 'E', 'u', 'U', 'r', 'R', false) ||
					delete_if_is_in_r2(text,/*ation*/'a', 'A', 't', 'T', 'i', 'I', 'o', 'O', 'n', 'N', false) )
				{
				if (length != text.length() )
					{
					ic_to_iqu(text);
					m_step_1_successful = true;
					}
				return;
				}
			else if (is_suffix(text,/*usion*/'u', 'U', 's', 'S', 'i', 'I', 'o', 'O', 'n', 'N') ||
					is_suffix(text,/*ution*/'u', 'U', 't', 'T', 'i', 'I', 'o', 'O', 'n', 'N') )
				{
				if (Parent::m_r2 <= static_cast<unsigned int>(text.length()-5) )
					{
					text.erase(text.end()-4, text.end() );
					update_r_sections(text);
					m_step_1_successful = true;
					}
				return;
				}
			else if (is_suffix(text,/*ences*/'e', 'E', 'n', 'N', 'c', 'C', 'e', 'E', 's', 'S') )
				{
				if (Parent::m_r2 <= static_cast<unsigned int>(text.length()-5) )
					{
					///todo better case sensitivity here
					text.replace(text.end()-3, text.end(), "t");
					update_r_sections(text);
					m_step_1_successful = true;
					}
				return;
				}
			else if (delete_if_is_in_r2(text,/*ables*/'a', 'A', 'b', 'B', 'l', 'L', 'e', 'E', 's', 'S', false) ||
					delete_if_is_in_r2(text,/*istes*/'i', 'I', 's', 'S', 't', 'T', 'e', 'E', 's', 'S', false) ||
					delete_if_is_in_r2(text,/*ismes*/'i', 'I', 's', 'S', 'm', 'M', 'e', 'E', 's', 'S', false) ||
					delete_if_is_in_r2(text,/*ances*/'a', 'A', 'n', 'N', 'c', 'C', 'e', 'E', 's', 'S', false) )
				{
				if (length != text.length() )
					{
					m_step_1_successful = true;
					}
				return;
				}
			else if (text.length() >= 5 &&
					(text[text.length()-5] == 'i' || text[text.length()-5] == 'I') &&
					(text[text.length()-4] == 'q' || text[text.length()-4] == 'Q') &&
					(text[text.length()-2] == 'e' || text[text.length()-2] == 'E') &&
					(text[text.length()-1] == 's' || text[text.length()-1] == 'S') &&
					is_either(text[text.length()-3], LOWER_U_HASH, UPPER_U_HASH) )
					{
					if (Parent::m_r2 <= text.length()-5)
						{
						text.erase(text.end()-5, text.end() );
						update_r_sections(text);
						}
					if (length != text.length() )
						{
						m_step_1_successful = true;
						}
					return;
					}
			else if (is_suffix(text,/*logie*/'l', 'L', 'o', 'O', 'g', 'G', 'i', 'I', 'e', 'E') )
				{
				if (Parent::m_r2 <= static_cast<unsigned int>(text.length()-5) )
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					m_step_1_successful = true;
					}
				return;
				}
			else if (delete_if_is_in_rv(text,/*ement*/'e', 'E', 'm', 'M', 'e', 'E', 'n', 'N', 't', 'T', false) )
				{
				if (delete_if_is_in_r2(text,/*iv*/'i', 'I', 'v', 'V', false) )
					{
					delete_if_is_in_r2(text, /*at*/'a', 'A', 't', 'T');
					}
				else if (is_suffix(text,/*eus*/'e', 'E', 'u', 'U', 's', 'S') )
					{
					if (Parent::m_r2 <= text.length()-3)
						{
						text.erase(text.length()-3);
						update_r_sections(text);
						}
					else if (Parent::m_r1 <= text.length()-3)
						{
						///todo better case sensitivity
						text[text.length()-1] = 'x';
						}
					}
				else if (delete_if_is_in_r2(text,/*abl*/'a', 'A', 'b', 'B', 'l', 'L') )
					{
					//NOOP
					}
				else if (text.length() >= 3 &&
					(text[text.length()-3] == 'i' || text[text.length()-3] == 'I') &&
					(text[text.length()-2] == 'q' || text[text.length()-2] == 'Q') &&
					is_either(text[text.length()-1], LOWER_U_HASH, UPPER_U_HASH) )
					{
					if (Parent::m_r2 <= text.length()-3)
						{
						text.erase(text.end()-3, text.end() );
						update_r_sections(text);
						}
					}
				else if (is_suffix_in_rv(text,/*ièr*/'i', 'I', 'è', 'È', 'r', 'R') )
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					}
				else if (Parent::m_rv <= static_cast<int>(text.length()-3) &&
					(text[text.length()-2] == 'è' || text[text.length()-2] == 'È') &&
					(text[text.length()-1] == 'r' || text[text.length()-1] == 'R') &&
					is_either(text[text.length()-3], LOWER_I_HASH, UPPER_I_HASH) )
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					}
				if (length != text.length() )
					{
					m_step_1_successful = true;
					}
				}
			else if (is_suffix(text,/*ments*/'m', 'M', 'e', 'E', 'n', 'N', 't', 'T', 's', 'S') )
				{
				//the proceeding vowel must also be n RV
				if (Parent::m_rv <= text.length()-6 &&
					string_util::is_one_of(text[text.length()-6], "aeiouyâàëéêèïîôûùAEIOUYÂÀËÉÊÈÏÎÔÛÙ") )
					{
					text.erase(text.end()-5, text.end() );
					update_r_sections(text);	
					}
				return;
				}
			else if (is_suffix(text,/*euses*/'e', 'E', 'u', 'U', 's', 'S', 'e', 'E', 's', 'S') )
				{
				if (Parent::m_r2 <= text.length()-5)
					{
					text.erase(text.end()-5, text.end() );
					update_r_sections(text);
					}
				else if (Parent::m_r1 <= text.length()-5)
					{
					///todo make this more case sensitive
					text.replace(text.end()-3, text.end(), "x");
					update_r_sections(text);
					}
				m_step_1_successful = true;
				}
			//4
			else if (is_suffix(text,/*euse*/'e', 'E', 'u', 'U', 's', 'S', 'e', 'E') )
				{
				if (Parent::m_r2 <= text.length()-4)
					{
					text.erase(text.end()-4, text.end() );
					update_r_sections(text);
					}
				else if (Parent::m_r1 <= text.length()-4)
					{
					///todo make this more case sensitive
					text.replace(text.end()-2, text.end(), "x");
					update_r_sections(text);
					}
				m_step_1_successful = true;
				}
			else if (is_suffix(text,/*ment*/'m', 'M', 'e', 'E', 'n', 'N', 't', 'T') )
				{
				//the proceeding vowel must also be n RV
				if (Parent::m_rv <= text.length()-5 &&
					string_util::is_one_of(text[text.length()-5], "aeiouyâàëéêèïîôûùAEIOUYÂÀËÉÊÈÏÎÔÛÙ") )
					{
					text.erase(text.end()-4, text.end() );
					update_r_sections(text);	
					}
				return;
				}
			else if (is_suffix(text,/*ence*/'e', 'E', 'n', 'N', 'c', 'C', 'e', 'E') )
				{
				if (Parent::m_r2 <= static_cast<unsigned int>(text.length()-4) )
					{
					///todo make this more case sensitive
					text.replace(text.end()-2, text.end(), "t");
					update_r_sections(text);
					m_step_1_successful = true;
					}
				return;
				}
			else if (delete_if_is_in_r2(text,/*ance*/'a', 'A', 'n', 'N', 'c', 'C', 'e', 'E', false) ||
					delete_if_is_in_r2(text,/*isme*/'i', 'I', 's', 'S', 'm', 'M', 'e', 'E', false) ||
					delete_if_is_in_r2(text,/*able*/'a', 'A', 'b', 'B', 'l', 'L', 'e', 'E', false) ||
					delete_if_is_in_r2(text,/*iste*/'i', 'I', 's', 'S', 't', 'T', 'e', 'E', false) )
				{
				if (length != text.length() )
					{
					m_step_1_successful = true;
					}
				return;
				}
			else if (text.length() >= 4 &&
					(text[text.length()-4] == 'i' || text[text.length()-4] == 'I') &&
					(text[text.length()-3] == 'q' || text[text.length()-3] == 'Q') &&
					(text[text.length()-1] == 'e' || text[text.length()-1] == 'E') &&
					is_either(text[text.length()-2], LOWER_U_HASH, UPPER_U_HASH) )
					{
					if (Parent::m_r2 <= text.length()-4)
						{
						text.erase(text.end()-4, text.end() );
						update_r_sections(text);
						}
					if (length != text.length() )
						{
						m_step_1_successful = true;
						}
					return;
					}
			else if (is_suffix(text,/*eaux*/'e', 'E', 'a', 'A', 'u', 'U', 'x', 'X') )
				{
				text.erase(text.end()-1, text.end() );
				update_r_sections(text);
				m_step_1_successful = true;
				return;
				}
			else if (delete_if_is_in_r2(text,/*ités*/'i', 'I', 't', 'T', 'é', 'É', 's', 'S', false) )
				{
				if (is_suffix(text,/*abil*/'a', 'A', 'b', 'B', 'i', 'I', 'l', 'L') )
					{
					if (Parent::m_r2 <= text.length()-4)
						{
						text.erase(text.length()-4);
						update_r_sections(text);
						}
					else
						{
						///todo make this more case sensitive
						text.replace(text.end()-2, text.end(), "l");
						}
					}
				else if (is_suffix(text,/*ic*/'i', 'I', 'c', 'C') )
					{
					if (Parent::m_r2 <= text.length()-2)
						{
						text.erase(text.length()-2);
						update_r_sections(text);
						}
					else
						{
						text.erase(text.length()-2);
						text += "iq";
						text += LOWER_U_HASH;
						update_r_sections(text);
						}
					}
				else
					{
					delete_if_is_in_r2(text,/*iv*/'i', 'I', 'v', 'V');
					}
				if (length != text.length() )
					{
					m_step_1_successful = true;
					}
				return;
				}
			else if (delete_if_is_in_r2(text,/*ives*/'i', 'I', 'v', 'V', 'e', 'E', 's', 'S', false) )
				{
				if (delete_if_is_in_r2(text,/*at*/'a', 'A', 't', 'T') )
					{
					ic_to_iqu(text);
					}
				if (length != text.length() )
					{
					m_step_1_successful = true;
					}
				return;
				}
			//3
			else if (delete_if_is_in_r2(text,/*ité*/'i', 'I', 't', 'T', 'é', 'É', false) )
				{
				if (is_suffix(text,/*abil*/'a', 'A', 'b', 'B', 'i', 'I', 'l', 'L') )
					{
					if (Parent::m_r2 <= text.length()-4)
						{
						text.erase(text.length()-4);
						update_r_sections(text);
						}
					else
						{
						///todo make this more case sensitive
						text.replace(text.end()-2, text.end(), "l");
						}
					}
				else if (is_suffix(text,/*ic*/'i', 'I', 'c', 'C') )
					{
					if (Parent::m_r2 <= text.length()-2)
						{
						text.erase(text.length()-2);
						update_r_sections(text);
						}
					else
						{
						text.erase(text.length()-2);
						text += "iq";
						text += LOWER_U_HASH;
						update_r_sections(text);
						}
					}
				else
					{
					delete_if_is_in_r2(text,/*iv*/'i', 'I', 'v', 'V');
					}
				if (length != text.length() )
					{
					m_step_1_successful = true;
					}
				return;
				}
			else if (delete_if_is_in_r2(text,/*eux*/'e', 'E', 'u', 'U', 'x', 'X', false) )
				{
				if (length != text.length() )
					{
					m_step_1_successful = true;
					}
				return;
				}
			else if (is_suffix(text,/*aux*/'a', 'A', 'u', 'U', 'x', 'X') )
				{
				if (Parent::m_r1 <= static_cast<unsigned int>(text.length()-3) )
					{
					///todo make this more case sensitive
					text.replace(text.end()-2, text.end(), "l");
					update_r_sections(text);
					m_step_1_successful = true;
					}
				return;
				}
			else if (delete_if_is_in_r2(text,/*ive*/'i', 'I', 'v', 'V', 'e', 'E', false) ||
					delete_if_is_in_r2(text,/*ifs*/'i', 'I', 'f', 'F', 's', 'S', false) )
				{
				if (delete_if_is_in_r2(text,/*at*/'a', 'A', 't', 'T', false) )
					{
					ic_to_iqu(text);
					}
				if (length != text.length() )
					{
					m_step_1_successful = true;
					}
				return;
				}
			//2
			else if (delete_if_is_in_r2(text,/*if*/'i', 'I', 'f', 'F', false) )
				{
				if (delete_if_is_in_r2(text,/*at*/'a', 'A', 't', 'T', false) )
					{
					ic_to_iqu(text);
					}
				if (length != text.length() )
					{
					m_step_1_successful = true;
					}
				return;
				}
			}
		/**In steps 2a and 2b all tests are confined to the RV region. 

		Do step 2a if either no ending was removed by step 1, or if one of endings amment, emment, ment, ments was found. 

		Search for the longest among the following suffixes and if found, delete if preceded by a non-vowel. 

			-îmes ît îtes i ie ies ir ira irai iraIent irais irait iras irent irez iriez irions irons iront is issaIent
			issais issait issant issante issantes issants isse issent isses issez issiez issions issons it 

		(Note that the non-vowel itself must also be in RV.)*/
		//---------------------------------------------
		void step_2a(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			if (Parent::m_rv <= static_cast<int>(text.length()-8) &&
				(text[text.length()-8] == 'i' || text[text.length()-8] == 'I') &&
				(text[text.length()-7] == 's' || text[text.length()-7] == 'S') &&
				(text[text.length()-6] == 's' || text[text.length()-6] == 'S') &&
				(text[text.length()-5] == 'a' || text[text.length()-5] == 'A') &&
				(text[text.length()-3] == 'e' || text[text.length()-3] == 'E') &&
				(text[text.length()-2] == 'n' || text[text.length()-2] == 'N') &&
				(text[text.length()-1] == 't' || text[text.length()-1] == 'T') &&
				is_either(text[text.length()-4], LOWER_I_HASH, UPPER_I_HASH) )
				{
				if (Parent::m_rv <= static_cast<int>(text.length()-7) &&
					!string_util::is_one_of(text[text.length()-9], "aeiouyâàëéêèïîôûùAEIOUYÂÀËÉÊÈÏÎÔÛÙ") )
					{
					text.erase(text.end()-8, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*issantes*/'i', 'I', 's', 'S', 's', 'S', 'a', 'A', 'n', 'N', 't', 'T', 'e', 'E', 's', 'S') )
				{
				if (Parent::m_rv <= static_cast<int>(text.length()-9) &&
					!string_util::is_one_of(text[text.length()-9], "aeiouyâàëéêèïîôûùAEIOUYÂÀËÉÊÈÏÎÔÛÙ") )
					{
					text.erase(text.end()-8, text.end() );
					return;
					}
				}
			else if (Parent::m_rv <= static_cast<int>(text.length()-7) &&
				(text[text.length()-7] == 'i' || text[text.length()-7] == 'I') &&
				(text[text.length()-6] == 'r' || text[text.length()-6] == 'R') &&
				(text[text.length()-5] == 'a' || text[text.length()-5] == 'A') &&
				(text[text.length()-3] == 'e' || text[text.length()-3] == 'E') &&
				(text[text.length()-2] == 'n' || text[text.length()-2] == 'N') &&
				(text[text.length()-1] == 't' || text[text.length()-1] == 'T') &&
				is_either(text[text.length()-4], LOWER_I_HASH, UPPER_I_HASH) )
				{
				if (Parent::m_rv <= static_cast<int>(text.length()-8) &&
					!string_util::is_one_of(text[text.length()-8], "aeiouyâàëéêèïîôûùAEIOUYÂÀËÉÊÈÏÎÔÛÙ") )
					{
					text.erase(text.end()-7, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*issante*/'i', 'I', 's', 'S', 's', 'S', 'a', 'A', 'n', 'N', 't', 'T', 'e', 'E') )
				{
				if (Parent::m_rv <= static_cast<int>(text.length()-8) &&
					!string_util::is_one_of(text[text.length()-8], "aeiouyâàëéêèïîôûùAEIOUYÂÀËÉÊÈÏÎÔÛÙ") )
					{
					text.erase(text.end()-7, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*issants*/'i', 'I', 's', 'S', 's', 'S', 'a', 'A', 'n', 'N', 't', 'T', 's', 'S') )
				{
				if (Parent::m_rv <= static_cast<int>(text.length()-8) &&
					!string_util::is_one_of(text[text.length()-8], "aeiouyâàëéêèïîôûùAEIOUYÂÀËÉÊÈÏÎÔÛÙ") )
					{
					text.erase(text.end()-7, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*issions*/'i', 'I', 's', 'S', 's', 'S', 'i', 'I', 'o', 'O', 'n', 'N', 's', 'S') )
				{
				if (Parent::m_rv <= static_cast<int>(text.length()-8) &&
					!string_util::is_one_of(text[text.length()-8], "aeiouyâàëéêèïîôûùAEIOUYÂÀËÉÊÈÏÎÔÛÙ") )
					{
					text.erase(text.end()-7, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*irions*/'i', 'I', 'r', 'R', 'i', 'I', 'o', 'O', 'n', 'N', 's', 'S') )
				{
				if (Parent::m_rv <= static_cast<int>(text.length()-7) &&
					!string_util::is_one_of(text[text.length()-7], "aeiouyâàëéêèïîôûùAEIOUYÂÀËÉÊÈÏÎÔÛÙ") )
					{
					text.erase(text.end()-6, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*issais*/'i', 'I', 's', 'S', 's', 'S', 'a', 'A', 'i', 'I', 's', 'S') )
				{
				if (Parent::m_rv <= static_cast<int>(text.length()-7) &&
					!string_util::is_one_of(text[text.length()-7], "aeiouyâàëéêèïîôûùAEIOUYÂÀËÉÊÈÏÎÔÛÙ") )
					{
					text.erase(text.end()-6, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*issait*/'i', 'I', 's', 'S', 's', 'S', 'a', 'A', 'i', 'I', 't', 'T') )
				{
				if (Parent::m_rv <= static_cast<int>(text.length()-7) &&
					!string_util::is_one_of(text[text.length()-7], "aeiouyâàëéêèïîôûùAEIOUYÂÀËÉÊÈÏÎÔÛÙ") )
					{
					text.erase(text.end()-6, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*issant*/'i', 'I', 's', 'S', 's', 'S', 'a', 'A', 'n', 'N', 't', 'T') )
				{
				if (Parent::m_rv <= static_cast<int>(text.length()-7) &&
					!string_util::is_one_of(text[text.length()-7], "aeiouyâàëéêèïîôûùAEIOUYÂÀËÉÊÈÏÎÔÛÙ") )
					{
					text.erase(text.end()-6, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*issent*/'i', 'I', 's', 'S', 's', 'S', 'e', 'E', 'n', 'N', 't', 'T') )
				{
				if (Parent::m_rv <= static_cast<int>(text.length()-7) &&
					!string_util::is_one_of(text[text.length()-7], "aeiouyâàëéêèïîôûùAEIOUYÂÀËÉÊÈÏÎÔÛÙ") )
					{
					text.erase(text.end()-6, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*issiez*/'i', 'I', 's', 'S', 's', 'S', 'i', 'I', 'e', 'E', 'z', 'Z') )
				{
				if (Parent::m_rv <= static_cast<int>(text.length()-7) &&
					!string_util::is_one_of(text[text.length()-7], "aeiouyâàëéêèïîôûùAEIOUYÂÀËÉÊÈÏÎÔÛÙ") )
					{
					text.erase(text.end()-6, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*issons*/'i', 'I', 's', 'S', 's', 'S', 'o', 'O', 'n', 'N', 's', 'S') )
				{
				if (Parent::m_rv <= static_cast<int>(text.length()-7) &&
					!string_util::is_one_of(text[text.length()-7], "aeiouyâàëéêèïîôûùAEIOUYÂÀËÉÊÈÏÎÔÛÙ") )
					{
					text.erase(text.end()-6, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*irais*/'i', 'I', 'r', 'R', 'a', 'A', 'i', 'I', 's', 'S') )
				{
				if (Parent::m_rv <= static_cast<int>(text.length()-6) &&
					!string_util::is_one_of(text[text.length()-6], "aeiouyâàëéêèïîôûùAEIOUYÂÀËÉÊÈÏÎÔÛÙ") )
					{
					text.erase(text.end()-5, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*irait*/'i', 'I', 'r', 'R', 'a', 'A', 'i', 'I', 't', 'T') )
				{
				if (Parent::m_rv <= static_cast<int>(text.length()-6) &&
					!string_util::is_one_of(text[text.length()-6], "aeiouyâàëéêèïîôûùAEIOUYÂÀËÉÊÈÏÎÔÛÙ") )
					{
					text.erase(text.end()-5, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*irent*/'i', 'I', 'r', 'R', 'e', 'E', 'n', 'N', 't', 'T') )
				{
				if (Parent::m_rv <= static_cast<int>(text.length()-6) &&
					!string_util::is_one_of(text[text.length()-6], "aeiouyâàëéêèïîôûùAEIOUYÂÀËÉÊÈÏÎÔÛÙ") )
					{
					text.erase(text.end()-5, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*iriez*/'i', 'I', 'r', 'R', 'i', 'I', 'e', 'E', 'z', 'Z') )
				{
				if (Parent::m_rv <= static_cast<int>(text.length()-6) &&
					!string_util::is_one_of(text[text.length()-6], "aeiouyâàëéêèïîôûùAEIOUYÂÀËÉÊÈÏÎÔÛÙ") )
					{
					text.erase(text.end()-5, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*irons*/'i', 'I', 'r', 'R', 'o', 'O', 'n', 'N', 's', 'S') )
				{
				if (Parent::m_rv <= static_cast<int>(text.length()-6) &&
					!string_util::is_one_of(text[text.length()-6], "aeiouyâàëéêèïîôûùAEIOUYÂÀËÉÊÈÏÎÔÛÙ") )
					{
					text.erase(text.end()-5, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*iront*/'i', 'I', 'r', 'R', 'o', 'O', 'n', 'N', 't', 'T') )
				{
				if (Parent::m_rv <= static_cast<int>(text.length()-6) &&
					!string_util::is_one_of(text[text.length()-6], "aeiouyâàëéêèïîôûùAEIOUYÂÀËÉÊÈÏÎÔÛÙ") )
					{
					text.erase(text.end()-5, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*isses*/'i', 'I', 's', 'S', 's', 'S', 'e', 'E', 's', 'S') )
				{
				if (Parent::m_rv <= static_cast<int>(text.length()-6) &&
					!string_util::is_one_of(text[text.length()-6], "aeiouyâàëéêèïîôûùAEIOUYÂÀËÉÊÈÏÎÔÛÙ") )
					{
					text.erase(text.end()-5, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*issez*/'i', 'I', 's', 'S', 's', 'S', 'e', 'E', 'z', 'Z') )
				{
				if (Parent::m_rv <= static_cast<int>(text.length()-6)&&
					!string_util::is_one_of(text[text.length()-6], "aeiouyâàëéêèïîôûùAEIOUYÂÀËÉÊÈÏÎÔÛÙ") )
					{
					text.erase(text.end()-5, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*îmes*/'î', 'Î', 'm', 'M', 'e', 'E', 's', 'S') )
				{
				if (Parent::m_rv <= static_cast<int>(text.length()-5) &&
					!string_util::is_one_of(text[text.length()-5], "aeiouyâàëéêèïîôûùAEIOUYÂÀËÉÊÈÏÎÔÛÙ") )
					{
					text.erase(text.end()-4, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*îtes*/'î', 'Î', 't', 'T', 'e', 'E', 's', 'S') )
				{
				if (Parent::m_rv <= static_cast<int>(text.length()-5) &&
					!string_util::is_one_of(text[text.length()-5], "aeiouyâàëéêèïîôûùAEIOUYÂÀËÉÊÈÏÎÔÛÙ") )
					{
					text.erase(text.end()-4, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*irai*/'i', 'I', 'r', 'R', 'a', 'A', 'i', 'I') )
				{
				if (Parent::m_rv <= static_cast<int>(text.length()-5) &&
					!string_util::is_one_of(text[text.length()-5], "aeiouyâàëéêèïîôûùAEIOUYÂÀËÉÊÈÏÎÔÛÙ") )
					{
					text.erase(text.end()-4, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*iras*/'i', 'I', 'r', 'R', 'a', 'A', 's', 'S') )
				{
				if (Parent::m_rv <= static_cast<int>(text.length()-5) &&
					!string_util::is_one_of(text[text.length()-5], "aeiouyâàëéêèïîôûùAEIOUYÂÀËÉÊÈÏÎÔÛÙ") )
					{
					text.erase(text.end()-4, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*irez*/'i', 'I', 'r', 'R', 'e', 'E', 'z', 'Z') )
				{
				if (Parent::m_rv <= static_cast<int>(text.length()-5) &&
					!string_util::is_one_of(text[text.length()-5], "aeiouyâàëéêèïîôûùAEIOUYÂÀËÉÊÈÏÎÔÛÙ") )
					{
					text.erase(text.end()-4, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*isse*/'i', 'I', 's', 'S', 's', 'S', 'e', 'E') )
				{
				if (Parent::m_rv <= static_cast<int>(text.length()-5) &&
					!string_util::is_one_of(text[text.length()-5], "aeiouyâàëéêèïîôûùAEIOUYÂÀËÉÊÈÏÎÔÛÙ") )
					{
					text.erase(text.end()-4, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*ies*/'i', 'I', 'e', 'E', 's', 'S') )
				{
				if (Parent::m_rv <= static_cast<int>(text.length()-4) &&
					!string_util::is_one_of(text[text.length()-4], "aeiouyâàëéêèïîôûùAEIOUYÂÀËÉÊÈÏÎÔÛÙ") )
					{
					text.erase(text.end()-3, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*ira*/'i', 'I', 'r', 'R', 'a', 'A') )
				{
				if (Parent::m_rv <= static_cast<int>(text.length()-4) &&
					!string_util::is_one_of(text[text.length()-4], "aeiouyâàëéêèïîôûùAEIOUYÂÀËÉÊÈÏÎÔÛÙ") )
					{
					text.erase(text.end()-3, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*ît*/'î', 'Î', 't', 'T') )
				{
				if (Parent::m_rv <= text.length()-3 &&
					!string_util::is_one_of(text[text.length()-3], "aeiouyâàëéêèïîôûùAEIOUYÂÀËÉÊÈÏÎÔÛÙ") )
					{
					text.erase(text.end()-2, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*ie*/'i', 'I', 'e', 'E') )
				{
				if (Parent::m_rv <= text.length()-3 &&
					!string_util::is_one_of(text[text.length()-3], "aeiouyâàëéêèïîôûùAEIOUYÂÀËÉÊÈÏÎÔÛÙ") )
					{
					text.erase(text.end()-2, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*ir*/'i', 'I', 'r', 'R') )
				{
				if (Parent::m_rv <= text.length()-3 &&
					!string_util::is_one_of(text[text.length()-3], "aeiouyâàëéêèïîôûùAEIOUYÂÀËÉÊÈÏÎÔÛÙ") )
					{
					text.erase(text.end()-2, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*is*/'i', 'I', 's', 'S') )
				{
				if (Parent::m_rv <= static_cast<int>(text.length()-3) &&
					!string_util::is_one_of(text[text.length()-3], "aeiouyâàëéêèïîôûùAEIOUYÂÀËÉÊÈÏÎÔÛÙ") )
					{
					text.erase(text.end()-2, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*it*/'i', 'I', 't', 'T') )
				{
				if (Parent::m_rv <= static_cast<int>(text.length()-3) &&
					!string_util::is_one_of(text[text.length()-3], "aeiouyâàëéêèïîôûùAEIOUYÂÀËÉÊÈÏÎÔÛÙ") )
					{
					text.erase(text.end()-2, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text, 'i', 'I') )
				{
				if (Parent::m_rv <= static_cast<int>(text.length()-2) &&
					!string_util::is_one_of(text[text.length()-2], "aeiouyâàëéêèïîôûùAEIOUYÂÀËÉÊÈÏÎÔÛÙ") )
					{
					text.erase(text.end()-1, text.end() );
					return;
					}
				}
			//only called if 2a fails to remove a suffix
			step_2b(text);
			}
		/**Do step 2b if step 2a was done, but failed to remove a suffix.
		Search for the longest among the following suffixes, and perform the action indicated. 

			-ions 
				-delete if in R2 

			-é ée ées és èrent er era erai eraIent erais erait eras erez eriez erions erons eront ez iez 
				-delete 

			-âmes ât âtes a ai aIent ais ait ant ante antes ants as asse assent asses assiez assions 
				-delete 
				-if preceded by e, delete 

		(Note that the e that may be deleted in this last step must also be in RV.) */
		//---------------------------------------------
		void step_2b(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			if (delete_if_is_in_rv(text,/*assions*/'a', 'A', 's', 'S', 's', 'S', 'i', 'I', 'o', 'O', 'n', 'N', 's', 'S', false) )
				{
				delete_if_is_in_rv(text, 'e', 'E');
				return;
				}
			else if (delete_if_is_in_rv(text,/*assent*/'a', 'A', 's', 'S', 's', 'S', 'e', 'E', 'n', 'N', 't', 'T', false) )
				{
				delete_if_is_in_rv(text, 'e', 'E');
				return;
				}
			else if (delete_if_is_in_rv(text,/*assiez*/'a', 'A', 's', 'S', 's', 'S', 'i', 'I', 'e', 'E', 'z', 'Z', false) )
				{
				delete_if_is_in_rv(text, 'e', 'E');
				return;
				}
			else if (Parent::m_rv <= static_cast<int>(text.length()-7) &&
				(text[text.length()-7] == 'e' || text[text.length()-7] == 'E') &&
				(text[text.length()-6] == 'r' || text[text.length()-6] == 'R') &&
				(text[text.length()-5] == 'a' || text[text.length()-5] == 'A') &&
				(text[text.length()-3] == 'e' || text[text.length()-3] == 'E') &&
				(text[text.length()-2] == 'n' || text[text.length()-2] == 'N') &&
				(text[text.length()-1] == 't' || text[text.length()-1] == 'T') &&
				is_either(text[text.length()-4], LOWER_I_HASH, UPPER_I_HASH) )
				{
				text.erase(text.end()-7, text.end() );
				return;
				}
			else if (delete_if_is_in_rv(text,/*erions*/'e', 'E', 'r', 'R', 'i', 'I', 'o', 'O', 'n', 'N', 's', 'S', false) )
				{
				return;
				}
			else if (Parent::m_rv <= static_cast<int>(text.length()-5) &&
				(text[text.length()-5] == 'a' || text[text.length()-5] == 'A') &&
				(text[text.length()-3] == 'e' || text[text.length()-3] == 'E') &&
				(text[text.length()-2] == 'n' || text[text.length()-2] == 'N') &&
				(text[text.length()-1] == 't' || text[text.length()-1] == 'T') &&
				is_either(text[text.length()-4], LOWER_I_HASH, UPPER_I_HASH) )
				{
				text.erase(text.end()-5, text.end() );
				delete_if_is_in_rv(text, 'e', 'E');
				return;
				}
			else if (delete_if_is_in_rv(text,/*antes*/'a', 'A', 'n', 'N', 't', 'T', 'e', 'E', 's', 'S', false) )
				{
				delete_if_is_in_rv(text, 'e', 'E');
				return;
				}
			else if (delete_if_is_in_rv(text,/*asses*/'a', 'A', 's', 'S', 's', 'S', 'e', 'E', 's', 'S', false) )
				{
				delete_if_is_in_rv(text, 'e', 'E');
				return;
				}
			else if (delete_if_is_in_rv(text,/*èrent*/'è', 'È', 'r', 'R', 'e', 'E', 'n', 'N', 't', 'T', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*erais*/'e', 'E', 'r', 'R', 'a', 'A', 'i', 'I', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*erait*/'e', 'E', 'r', 'R', 'a', 'A', 'i', 'I', 't', 'T', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*eriez*/'e', 'E', 'r', 'R', 'i', 'I', 'e', 'E', 'z', 'Z', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*erons*/'e', 'E', 'r', 'R', 'o', 'O', 'n', 'N', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*eront*/'e', 'E', 'r', 'R', 'o', 'O', 'n', 'N', 't', 'T', false) )
				{
				return;
				}
			else if (is_suffix_in_r1(text,/*ions*/'i', 'I', 'o', 'O', 'n', 'N', 's', 'S') &&
				delete_if_is_in_r2(text,/*ions*/'i', 'I', 'o', 'O', 'n', 'N', 's', 'S') )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*âmes*/'â', 'Â', 'm', 'M', 'e', 'E', 's', 'S', false) )
				{
				delete_if_is_in_rv(text, 'e', 'E');
				return;
				}
			else if (delete_if_is_in_rv(text,/*âtes*/'â', 'Â', 't', 'T', 'e', 'E', 's', 'S', false) )
				{
				delete_if_is_in_rv(text, 'e', 'E');
				return;
				}
			else if (delete_if_is_in_rv(text,/*ante*/'a', 'A', 'n', 'N', 't', 'T', 'e', 'E', false) )
				{
				delete_if_is_in_rv(text, 'e', 'E');
				return;
				}
			else if (delete_if_is_in_rv(text,/*ants*/'a', 'A', 'n', 'N', 't', 'T', 's', 'S', false) )
				{
				delete_if_is_in_rv(text, 'e', 'E');
				return;
				}
			else if (delete_if_is_in_rv(text,/*asse*/'a', 'A', 's', 'S', 's', 'S', 'e', 'E', false) )
				{
				delete_if_is_in_rv(text, 'e', 'E');
				return;
				}
			else if (delete_if_is_in_rv(text,/*erai*/'e', 'E', 'r', 'R', 'a', 'A', 'i', 'I', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*eras*/'e', 'E', 'r', 'R', 'a', 'A', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*erez*/'e', 'E', 'r', 'R', 'e', 'E', 'z', 'Z', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ais*/'a', 'A', 'i', 'I', 's', 'S', false) )
				{
				delete_if_is_in_rv(text, 'e', 'E');
				return;
				}
			else if (delete_if_is_in_rv(text,/*ait*/'a', 'A', 'i', 'I', 't', 'T', false) )
				{
				delete_if_is_in_rv(text, 'e', 'E');
				return;
				}
			else if (delete_if_is_in_rv(text,/*ant*/'a', 'A', 'n', 'N', 't', 'T', false) )
				{
				delete_if_is_in_rv(text, 'e', 'E');
				return;
				}
			else if (delete_if_is_in_rv(text,/*ées*/'é', 'É', 'e', 'E', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*era*/'e', 'E', 'r', 'R', 'a', 'A', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*iez*/'i', 'I', 'e', 'E', 'z', 'Z', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ât*/'â', 'Â', 't', 'T', false) )
				{
				delete_if_is_in_rv(text, 'e', 'E');
				return;
				}
			else if (delete_if_is_in_rv(text,/*ai*/'a', 'A', 'i', 'I', false) )
				{
				delete_if_is_in_rv(text,'e', 'E');
				return;
				}
			else if (delete_if_is_in_rv(text,/*as*/'a', 'A', 's', 'S', false) )
				{
				delete_if_is_in_rv(text, 'e', 'E');
				return;
				}
			else if (delete_if_is_in_rv(text,/*ée*/'é', 'É', 'e', 'E', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*és*/'é', 'É', 's', 'S', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*er*/'e', 'E', 'r', 'R', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ez*/'e', 'E', 'z', 'Z', false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text, 'a', 'A', false) )
				{
				delete_if_is_in_rv(text, 'e', 'E');
				return;
				}
			else if (delete_if_is_in_rv(text,/*é*/'é', 'É', false) )
				{
				return;
				}
			}
		/**If the last step to be obeyed - either step 1, 2a or 2b - altered the word, do step 3 
		Replace final Y with i or final ç with c */
		//---------------------------------------------
		void step_3(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			if (text[text.length()-1] == LOWER_Y_HASH)
				{
				text[text.length()-1] = 'i';
				}
			else if (text[text.length()-1] == UPPER_Y_HASH)
				{
				text[text.length()-1] = 'I';
				}
			else if (text[text.length()-1] == sign_char(199) )
				{
				text[text.length()-1] = 'C';
				}
			else if (text[text.length()-1] == sign_char(231) )
				{
				text[text.length()-1] = 'c';
				}
			}
		/**Alternatively, if the last step to be obeyed did not alter the word, do step 4

		If the word ends s, not preceded by a, i, o, u, è or s, delete it. 

		In the rest of step 4, all tests are confined to the RV region. 

		Search for the longest among the following suffixes, and perform the action indicated. 

			-ion 
				-delete if in R2 and preceded by s or t 

			-ier ière Ier Ière 
				-replace with i 

			-e 
				-delete 

			-ë 
				-if preceded by gu, delete 

		(So note that ion is removed only when it is in R2 - as well as being in RV -
		and preceded by s or t which must be in RV.)*/
		//---------------------------------------------
		void step_4(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			if ((text[text.length()-1] == 's' || text[text.length()-1] == 'S') &&
				!string_util::is_one_of(text[text.length()-2], "aiouèsAIOUÈS") )
				{
				text.erase(text.end()-1, text.end() );
				update_r_sections(text);
				}

			if (is_suffix_in_rv(text,/*ière*/'i', 'I', 'è', 'È', 'r', 'R', 'e', 'E') )
				{
				text.erase(text.end()-3, text.end() );
				text[text.length()-1] = 'i';
				update_r_sections(text);
				return;
				}
			else if (Parent::m_rv <= static_cast<int>(text.length()-4) &&
				(text[text.length()-3] == 'è' || text[text.length()-3] == 'È') &&
				(text[text.length()-2] == 'r' || text[text.length()-2] == 'R') &&
				(text[text.length()-1] == 'e' || text[text.length()-1] == 'E') &&
				is_either(text[text.length()-4], LOWER_I_HASH, UPPER_I_HASH) )
				{
				text.erase(text.end()-3, text.end() );
				text[text.length()-1] = 'i';
				update_r_sections(text);
				return;
				}
			else if (is_suffix_in_rv(text,/*ier*/'i', 'I', 'e', 'E', 'r', 'R') )
				{
				text.erase(text.end()-2, text.end() );
				text[text.length()-1] = 'i';
				update_r_sections(text);
				return;
				}
			else if (Parent::m_rv <= static_cast<int>(text.length()-3) &&
				(text[text.length()-2] == 'e' || text[text.length()-2] == 'E') &&
				(text[text.length()-1] == 'r' || text[text.length()-1] == 'R') &&
				is_either(text[text.length()-3], LOWER_I_HASH, UPPER_I_HASH) )
				{
				text.erase(text.end()-2, text.end() );
				text[text.length()-1] = 'i';
				update_r_sections(text);
				return;
				}
			else if (is_suffix_in_rv(text,/*sion*/'s', 'S', 'i', 'I', 'o', 'O', 'n', 'N') ||
				is_suffix_in_rv(text,/*tion*/'t', 'T', 'i', 'I', 'o', 'O', 'n', 'N') )
				{
				if (Parent::m_r2 <= text.length()-3)
					{
					text.erase(text.end()-3, text.end() );
					update_r_sections(text);
					}
				return;
				}
			else if (is_suffix_in_rv(text,/*ë*/'ë', 'Ë') )
				{
				if (text.length() >= 3 &&
					(is_either(text[text.length()-3], 'g', 'G') &&
					is_either(text[text.length()-2], 'u', 'U') ) )
					{
					text.erase(text.end()-1, text.end() );
					}
				return;
				}
			else if (delete_if_is_in_rv(text, 'e', 'E') )
				{
				return;
				}
			}
		///If the word ends enn, onn, ett, ell or eill, delete the last letter 
		//---------------------------------------------
		void step_5(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			if (is_suffix(text,/*enn*/'e', 'E', 'n', 'N', 'n', 'N') ||
				is_suffix(text,/*onn*/'o', 'O', 'n', 'N', 'n', 'N') ||
				is_suffix(text,/*ett*/'e', 'E', 't', 'T', 't', 'T') ||
				is_suffix(text,/*ell*/'e', 'E', 'l', 'L', 'l', 'L') ||
				is_suffix(text,/*eill*/'e', 'E', 'i', 'I', 'l', 'L', 'l', 'L') )
				{
				text.erase(text.end()-1, text.end() );
				update_r_sections(text);
				}
			}
		///If the words ends é or è followed by at least one non-vowel, remove the accent from the e. 
		//---------------------------------------------
		void step_6(std::basic_string<Tchar_type, Tchar_traits>& text)
			{
			size_t last_e = text.find_last_of("éèÉÈ");
			size_t last_vowel = text.find_last_of("aeiouyâàëéêèïîôûùAEIOUYÂÀËÉÊÈÏÎÔÛÙ");
			size_t last_consonant = text.find_last_not_of("aeiouyâàëéêèïîôûùAEIOUYÂÀËÉÊÈÏÎÔÛÙ");
			if (last_e == std::basic_string<Tchar_type, Tchar_traits>::npos ||
				last_consonant == std::basic_string<Tchar_type, Tchar_traits>::npos)
				{
				return;
				}
			else if (last_e >= last_vowel &&
					last_e < last_consonant)
				{
				text[last_e] = 'e';
				}
			}
		//internal data specific to French stemmer
		bool m_step_1_successful;
		};
	}

#endif //__FRENCH_STEM_H__

/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "punctuation_en.hpp"
#include "punctuation_utils.hpp"
#include "strus/errorBufferInterface.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include <cstring>

using namespace strus;

#undef STRUS_LOWLEVEL_DEBUG

static const char g_abbrevList[] = 
	"abbrev\0"
	"abd\0"
	"aberd\0"
	"aberdeensh\0"
	"abl\0"
	"abol\0"
	"aborig\0"
	"abp\0"
	"abr\0"
	"abridg\0"
	"abridgem\0"
	"absol\0"
	"abst\0"
	"abstr\0"
	"acad\0"
	"acc\0"
	"accept\0"
	"accomm\0"
	"accompl\0"
	"accs\0"
	"acct\0"
	"accts\0"
	"accus\0"
	"achievem\0"
	"ad\0"
	"add\0"
	"addit\0"
	"addr\0"
	"adj\0"
	"adjs\0"
	"adm\0"
	"admin\0"
	"admir\0"
	"admon\0"
	"admonit\0"
	"adv\0"
	"advancem\0"
	"advb\0"
	"advert\0"
	"advoc\0"
	"advs\0"
	"advt\0"
	"advts\0"
	"aerodynam\0"
	"aeronaut\0"
	"aff\0"
	"affect\0"
	"afr\0"
	"agric\0"
	"agst\0"
	"alch\0"
	"alg\0"
	"alleg\0"
	"allit\0"
	"alm\0"
	"alph\0"
	"alt\0"
	"amer\0"
	"anal\0"
	"analyt\0"
	"anat\0"
	"anc\0"
	"anecd\0"
	"ang\0"
	"angl\0"
	"anglo\0"
	"anim\0"
	"ann\0"
	"anniv\0"
	"annot\0"
	"anon\0"
	"answ\0"
	"ant\0"
	"anthrop\0"
	"anthropol\0"
	"antiq\0"
	"aphet\0"
	"apoc\0"
	"apol\0"
	"app\0"
	"appl\0"
	"applic\0"
	"appos\0"
	"apr\0"
	"arab\0"
	"arb\0"
	"arch\0"
	"archaeol\0"
	"archipel\0"
	"archit\0"
	"argt\0"
	"arith\0"
	"arithm\0"
	"arrangem\0"
	"art\0"
	"artic\0"
	"artific\0"
	"artill\0"
	"ashm\0"
	"assemb\0"
	"assoc\0"
	"assyriol\0"
	"astr\0"
	"astrol\0"
	"astron\0"
	"astronaut\0"
	"att\0"
	"attrib\0"
	"aug\0"
	"austral\0"
	"auth\0"
	"autobiog\0"
	"autobiogr\0"
	"ayrsh\0"
	"bacteriol\0"
	"bedford\0"
	"bedfordsh\0"
	"bef\0"
	"belg\0"
	"berks\0"
	"berksh\0"
	"berw\0"
	"berwicksh\0"
	"betw\0"
	"bibliogr\0"
	"biochem\0"
	"biog\0"
	"biogr\0"
	"biol\0"
	"bk\0"
	"bks\0"
	"bord\0"
	"bot\0"
	"bp\0"
	"braz\0"
	"brit\0"
	"bucks\0"
	"build\0"
	"bull\0"
	"bur\0"
	"cal\0"
	"calc\0"
	"calend\0"
	"calif\0"
	"calligr\0"
	"camb\0"
	"cambr\0"
	"campanol\0"
	"canad\0"
	"canterb\0"
	"capt\0"
	"cartogr\0"
	"catal\0"
	"catech\0"
	"cath\0"
	"cent\0"
	"ceram\0"
	"cert\0"
	"certif\0"
	"cf\0"
	"ch\0"
	"chamb\0"
	"char\0"
	"charac\0"
	"chas\0"
	"chem\0"
	"chesh\0"
	"chr\0"
	"chron\0"
	"chronol\0"
	"chrons\0"
	"cinematogr\0"
	"circ\0"
	"civ\0"
	"cl\0"
	"class\0"
	"classif\0"
	"climatol\0"
	"clin\0"
	"cogn\0"
	"col\0"
	"coll\0"
	"collect\0"
	"colloq\0"
	"coloss\0"
	"com\0"
	"comb\0"
	"combs\0"
	"comm\0"
	"commandm\0"
	"commend\0"
	"commerc\0"
	"commiss\0"
	"commonw\0"
	"communic\0"
	"comp\0"
	"compan\0"
	"compar\0"
	"compend\0"
	"compl\0"
	"compos\0"
	"conc\0"
	"conch\0"
	"concl\0"
	"concr\0"
	"conf\0"
	"confid\0"
	"confl\0"
	"confut\0"
	"congr\0"
	"congreg\0"
	"congress\0"
	"conj\0"
	"conn\0"
	"cons\0"
	"consc\0"
	"consecr\0"
	"consid\0"
	"consol\0"
	"const\0"
	"constit\0"
	"constr\0"
	"contemp\0"
	"contempl\0"
	"contempt\0"
	"contend\0"
	"content\0"
	"contin\0"
	"contr\0"
	"contradict\0"
	"contrib\0"
	"controv\0"
	"conv\0"
	"convent\0"
	"conversat\0"
	"convoc\0"
	"cor\0"
	"cornw\0"
	"coron\0"
	"corr\0"
	"corresp\0"
	"counc\0"
	"courtsh\0"
	"cpd\0"
	"craniol\0"
	"craniom\0"
	"crim\0"
	"crit\0"
	"crt\0"
	"crts\0"
	"cryptogr\0"
	"crystallogr\0"
	"ct\0"
	"cumb\0"
	"cumberld\0"
	"cumbld\0"
	"cycl\0"
	"cytol\0"
	"dan\0"
	"dat\0"
	"dau\0"
	"deb\0"
	"dec\0"
	"declar\0"
	"ded\0"
	"def\0"
	"deliv\0"
	"dem\0"
	"demonstr\0"
	"dep\0"
	"depred\0"
	"depredat\0"
	"dept\0"
	"derbysh\0"
	"deriv\0"
	"derog\0"
	"descr\0"
	"deut\0"
	"devel\0"
	"devonsh\0"
	"dial\0"
	"dict\0"
	"diffic\0"
	"dim\0"
	"direct\0"
	"dis\0"
	"disc\0"
	"discipl\0"
	"discov\0"
	"discrim\0"
	"discuss\0"
	"diss\0"
	"dist\0"
	"distemp\0"
	"distill\0"
	"distrib\0"
	"div\0"
	"divers\0"
	"dk\0"
	"doc\0"
	"doctr\0"
	"domest\0"
	"dr\0"
	"durh\0"
	"dyslog\0"
	"east\0"
	"eccl\0"
	"eccles\0"
	"ecclus\0"
	"ecol\0"
	"econ\0"
	"ed\0"
	"edin\0"
	"edinb\0"
	"educ\0"
	"edw\0"
	"egypt\0"
	"egyptol\0"
	"electr\0"
	"elem\0"
	"eliz\0"
	"elizab\0"
	"ellipt\0"
	"emb\0"
	"embryol\0"
	"emph\0"
	"encycl\0"
	"eng\0"
	"engin\0"
	"englishw\0"
	"enq\0"
	"ent\0"
	"enthus\0"
	"entom\0"
	"entomol\0"
	"enzymol\0"
	"ep\0"
	"eph\0"
	"ephes\0"
	"epil\0"
	"episc\0"
	"epist\0"
	"epit\0"
	"equip\0"
	"erron\0"
	"esd\0"
	"esp\0"
	"ess\0"
	"essent\0"
	"establ\0"
	"esth\0"
	"ethnol\0"
	"etym\0"
	"etymol\0"
	"euphem\0"
	"eval\0"
	"evang\0"
	"even\0"
	"evid\0"
	"evol\0"
	"ex\0"
	"exalt\0"
	"exam\0"
	"exc\0"
	"exch\0"
	"exec\0"
	"exerc\0"
	"exhib\0"
	"exod\0"
	"exped\0"
	"exper\0"
	"explan\0"
	"explic\0"
	"explor\0"
	"expos\0"
	"ezek\0"
	"fab\0"
	"fam\0"
	"famil\0"
	"farew\0"
	"feb\0"
	"fem\0"
	"ff\0"
	"fifesh\0"
	"fig\0"
	"fl\0"
	"footpr\0"
	"forfarsh\0"
	"fortif\0"
	"fortn\0"
	"found\0"
	"fr\0"
	"fragm\0"
	"fratern\0"
	"freq\0"
	"friendsh\0"
	"fund\0"
	"furnit\0"
	"fut\0"
	"gal\0"
	"gard\0"
	"gastron\0"
	"gaz\0"
	"gd\0"
	"gen\0"
	"geo\0"
	"geog\0"
	"geogr\0"
	"geol\0"
	"geom\0"
	"geomorphol\0"
	"ger\0"
	"gerund\0"
	"glac\0"
	"glasg\0"
	"glos\0"
	"gloss\0"
	"glouc\0"
	"gloucestersh\0"
	"gosp\0"
	"gov\0"
	"govt\0"
	"gr\0"
	"gram\0"
	"gramm\0"
	"gt\0"
	"gynaecol\0"
	"hab\0"
	"haematol\0"
	"hag\0"
	"hampsh\0"
	"handbk\0"
	"hants\0"
	"heb\0"
	"hebr\0"
	"hen\0"
	"her\0"
	"herb\0"
	"heref\0"
	"hereford\0"
	"herefordsh\0"
	"hertfordsh\0"
	"hierogl\0"
	"hist\0"
	"histol\0"
	"hom\0"
	"horol\0"
	"hort\0"
	"hos\0"
	"hosp\0"
	"househ\0"
	"housek\0"
	"husb\0"
	"hydraul\0"
	"hydrol\0"
	"ichth\0"
	"icthyol\0"
	"ideol\0"
	"idol\0"
	"illustr\0"
	"imag\0"
	"imit\0"
	"immunol\0"
	"imp\0"
	"imperf\0"
	"impers\0"
	"impf\0"
	"impr\0"
	"improp\0"
	"inaug\0"
	"inclos\0"
	"ind\0"
	"indef\0"
	"indic\0"
	"indir\0"
	"industr\0"
	"infin\0"
	"infl\0"
	"innoc\0"
	"inorg\0"
	"inq\0"
	"inst\0"
	"instr\0"
	"int\0"
	"intell\0"
	"intellect\0"
	"interc\0"
	"interj\0"
	"interl\0"
	"internat\0"
	"interpr\0"
	"interrog\0"
	"intr\0"
	"intrans\0"
	"intro\0"
	"introd\0"
	"inv\0"
	"invent\0"
	"invert\0"
	"invertebr\0"
	"investig\0"
	"investm\0"
	"invoc\0"
	"ir\0"
	"irel\0"
	"iron\0"
	"irreg\0"
	"isa\0"
	"isl\0"
	"ital\0"
	"jahrb\0"
	"jam\0"
	"jan\0"
	"jap\0"
	"jas\0"
	"jer\0"
	"joc\0"
	"josh\0"
	"jrnl\0"
	"jrnls\0"
	"jud\0"
	"judg\0"
	"jul\0"
	"jun\0"
	"jurisd\0"
	"jurisdict\0"
	"jurispr\0"
	"justif\0"
	"justific\0"
	"kent\0"
	"kgs\0"
	"kingd\0"
	"knowl\0"
	"kpr\0"
	"lab\0"
	"lam\0"
	"lament\0"
	"lanc\0"
	"lancash\0"
	"lancs\0"
	"lang\0"
	"langs\0"
	"lat\0"
	"ld\0"
	"lds\0"
	"lect\0"
	"leechd\0"
	"leg\0"
	"leicest\0"
	"leicester\0"
	"leicestersh\0"
	"leics\0"
	"let\0"
	"lett\0"
	"lev\0"
	"lex\0"
	"libr\0"
	"limnol\0"
	"lincolnsh\0"
	"lincs\0"
	"ling\0"
	"linn\0"
	"lit\0"
	"lithogr\0"
	"lithol\0"
	"liturg\0"
	"ll\0"
	"lond\0"
	"macc\0"
	"mach\0"
	"mag\0"
	"magn\0"
	"mal\0"
	"man\0"
	"managem\0"
	"manch\0"
	"manip\0"
	"manuf\0"
	"mar\0"
	"masc\0"
	"mass\0"
	"math\0"
	"matt\0"
	"meas\0"
	"measurem\0"
	"mech\0"
	"med\0"
	"medit\0"
	"mem\0"
	"merc\0"
	"merch\0"
	"metall\0"
	"metallif\0"
	"metallogr\0"
	"metamorph\0"
	"metaph\0"
	"metaphor\0"
	"meteorol\0"
	"meth\0"
	"metr\0"
	"metrop\0"
	"mex\0"
	"mic\0"
	"mich\0"
	"microbiol\0"
	"microsc\0"
	"midl\0"
	"mil\0"
	"milit\0"
	"min\0"
	"mineral\0"
	"misc\0"
	"miscell\0"
	"mispr\0"
	"mod\0"
	"monum\0"
	"morphol\0"
	"ms\0"
	"mss\0"
	"mt\0"
	"mtg\0"
	"mts\0"
	"munic\0"
	"munif\0"
	"munim\0"
	"mus\0"
	"myst\0"
	"myth\0"
	"mythol\0"
	"nah\0"
	"narr\0"
	"narrat\0"
	"nat\0"
	"naut\0"
	"nav\0"
	"navig\0"
	"neh\0"
	"neighb\0"
	"nerv\0"
	"neurol\0"
	"neurosurg\0"
	"newc\0"
	"newspr\0"
	"no\0"
	"nom\0"
	"nonce\0"
	"non\0"
	"nonconf\0"
	"norf\0"
	"north\0"
	"northamptonsh\0"
	"northants\0"
	"northumb\0"
	"northumbld\0"
	"northumbr\0"
	"norw\0"
	"norweg\0"
	"notts\0"
	"nov\0"
	"ns\0"
	"nucl\0"
	"num\0"
	"numism\0"
	"obad\0"
	"obed\0"
	"obj\0"
	"obl\0"
	"obs\0"
	"observ\0"
	"obstet\0"
	"obstetr\0"
	"occas\0"
	"occup\0"
	"occurr\0"
	"oceanogr\0"
	"oct\0"
	"off\0"
	"offic\0"
	"okla\0"
	"ont\0"
	"ophthalm\0"
	"ophthalmol\0"
	"opp\0"
	"oppress\0"
	"opt\0"
	"orac\0"
	"ord\0"
	"org\0"
	"organ\0"
	"orig\0"
	"orkn\0"
	"ornith\0"
	"ornithol\0"
	"orthogr\0"
	"outl\0"
	"oxf\0"
	"oxfordsh\0"
	"oxon\0"
	"pa\0"
	"palaeobot\0"
	"palaeogr\0"
	"palaeont\0"
	"palaeontol\0"
	"paraphr\0"
	"parasitol\0"
	"parl\0"
	"parnass\0"
	"pass\0"
	"path\0"
	"pathol\0"
	"peculat\0"
	"penins\0"
	"perf\0"
	"perh\0"
	"periodontol\0"
	"pers\0"
	"persec\0"
	"personif\0"
	"perthsh\0"
	"pet\0"
	"petrogr\0"
	"petrol\0"
	"pf\0"
	"pharm\0"
	"pharmaceut\0"
	"pharmacol\0"
	"phil\0"
	"philad\0"
	"philem\0"
	"philipp\0"
	"philol\0"
	"philos\0"
	"phoen\0"
	"phonet\0"
	"phonol\0"
	"photog\0"
	"photogr\0"
	"phr\0"
	"phrenol\0"
	"phys\0"
	"physiogr\0"
	"physiol\0"
	"pict\0"
	"pl\0"
	"plur\0"
	"poet\0"
	"pol\0"
	"polit\0"
	"polytechn\0"
	"pop\0"
	"porc\0"
	"port\0"
	"poss\0"
	"posth\0"
	"postm\0"
	"pott\0"
	"ppl\0"
	"pple\0"
	"pples\0"
	"pr\0"
	"pract\0"
	"prec\0"
	"pred\0"
	"predic\0"
	"predict\0"
	"pref\0"
	"preh\0"
	"prehist\0"
	"prep\0"
	"prerog\0"
	"pres\0"
	"presb\0"
	"preserv\0"
	"prim\0"
	"princ\0"
	"print\0"
	"priv\0"
	"prob\0"
	"probab\0"
	"probl\0"
	"proc\0"
	"prod\0"
	"prol\0"
	"pron\0"
	"pronunc\0"
	"prop\0"
	"propr\0"
	"pros\0"
	"prov\0"
	"provid\0"
	"provinc\0"
	"provis\0"
	"ps\0"
	"psych\0"
	"psychoanal\0"
	"psychoanalyt\0"
	"psychol\0"
	"psychopathol\0"
	"pt\0"
	"publ\0"
	"purg\0"
	"qld\0"
	"quot\0"
	"quots\0"
	"radiol\0"
	"reas\0"
	"reb\0"
	"rebell\0"
	"rec\0"
	"reclam\0"
	"recoll\0"
	"redempt\0"
	"redupl\0"
	"ref\0"
	"refash\0"
	"refl\0"
	"refus\0"
	"refut\0"
	"reg\0"
	"regic\0"
	"regist\0"
	"regr\0"
	"rel\0"
	"relig\0"
	"reminisc\0"
	"remonstr\0"
	"renfrewsh\0"
	"rep\0"
	"repr\0"
	"reprod\0"
	"rept\0"
	"repub\0"
	"res\0"
	"resid\0"
	"ret\0"
	"retrosp\0"
	"rev\0"
	"revol\0"
	"rhet\0"
	"rich\0"
	"rom\0"
	"ross\0"
	"roxb\0"
	"roy\0"
	"rudim\0"
	"russ\0"
	"sam\0"
	"sask\0"
	"sat\0"
	"sax\0"
	"sc\0"
	"scand\0"
	"sch\0"
	"sci\0"
	"scot\0"
	"scotl\0"
	"script\0"
	"sculpt\0"
	"seismol\0"
	"sel\0"
	"select\0"
	"sept\0"
	"ser\0"
	"serm\0"
	"sess\0"
	"settlem\0"
	"sev\0"
	"shakes\0"
	"shaks\0"
	"sheph\0"
	"shetl\0"
	"shropsh\0"
	"sing\0"
	"soc\0"
	"sociol\0"
	"sol\0"
	"som\0"
	"sonn\0"
	"south\0"
	"sp\0"
	"span\0"
	"spec\0"
	"specif\0"
	"specim\0"
	"spectrosc\0"
	"ss\0"
	"st\0"
	"staff\0"
	"stafford\0"
	"staffordsh\0"
	"staffs\0"
	"stand\0"
	"stat\0"
	"statist\0"
	"str\0"
	"stratigr\0"
	"struct\0"
	"stud\0"
	"subj\0"
	"subjunct\0"
	"subord\0"
	"subscr\0"
	"subscript\0"
	"subseq\0"
	"subst\0"
	"suff\0"
	"superl\0"
	"suppl\0"
	"supplic\0"
	"suppress\0"
	"surg\0"
	"surv\0"
	"sus\0"
	"syll\0"
	"symmetr\0"
	"symp\0"
	"syst\0"
	"taxon\0"
	"techn\0"
	"technol\0"
	"tel\0"
	"telecomm\0"
	"telegr\0"
	"teleph\0"
	"teratol\0"
	"terminol\0"
	"terrestr\0"
	"test\0"
	"textbk\0"
	"theat\0"
	"theatr\0"
	"theol\0"
	"theoret\0"
	"thermonucl\0"
	"thes\0"
	"thess\0"
	"tim\0"
	"tit\0"
	"topogr\0"
	"tr\0"
	"trad\0"
	"trag\0"
	"trans\0"
	"transf\0"
	"transl\0"
	"transubstant\0"
	"trav\0"
	"treas\0"
	"treat\0"
	"treatm\0"
	"trib\0"
	"trig\0"
	"trigonom\0"
	"trop\0"
	"troub\0"
	"troubl\0"
	"typog\0"
	"typogr\0"
	"ult\0"
	"univ\0"
	"unkn\0"
	"unnat\0"
	"unoffic\0"
	"unstr\0"
	"urin\0"
	"usu\0"
	"utilit\0"
	"va\0"
	"vac\0"
	"valedict\0"
	"var\0"
	"varr\0"
	"vars\0"
	"vb\0"
	"vbl\0"
	"vbs\0"
	"veg\0"
	"venet\0"
	"vertebr\0"
	"vet\0"
	"vic\0"
	"vict\0"
	"vind\0"
	"vindic\0"
	"virg\0"
	"virol\0"
	"viz\0"
	"voc\0"
	"vocab\0"
	"vol\0"
	"vols\0"
	"voy\0"
	"vulg\0"
	"warwicksh\0"
	"wd\0"
	"west\0"
	"westm\0"
	"westmld\0"
	"westmorld\0"
	"westmrld\0"
	"will\0"
	"wilts\0"
	"wiltsh\0"
	"wis\0"
	"wisd\0"
	"wk\0"
	"wkly\0"
	"wks\0"
	"wonderf\0"
	"worc\0"
	"worcestersh\0"
	"worcs\0"
	"writ\0"
	"yearbk\0"
	"yng\0"
	"yorks\0"
	"yorksh\0"
	"yr\0"
	"yrs\0"
	"zech\0"
	"zeitschr\0"
	"zeph\0"
	"zoogeogr\0"
	"zool\0"
;

PunctuationTokenizerInstance_en::PunctuationTokenizerInstance_en(
		const char* punctuationCharList,
		ErrorBufferInterface* errorhnd)
	:m_punctuation_char(punctuationCharList?(punctuationCharList[0]?punctuationCharList:"."):":.;,!?()-")
	,m_punctuation_charlist(punctuationCharList?(punctuationCharList[0]?punctuationCharList:"."):":.;,!?()-")
	,m_errorhnd(errorhnd)
{
	char const* cc = g_abbrevList;
	char const* ee = std::strchr( cc, '\0');
	int idx = 1;
	for (; ee[1]; cc=ee+1,ee=std::strchr( cc, '\0'))
	{
		m_abbrevDict.set( cc, idx);
	}
	m_abbrevDict.set( cc, idx);
}


std::vector<analyzer::Token>
	PunctuationTokenizerInstance_en::tokenize(
		const char* src, std::size_t srcsize) const
{
	try
	{
		std::vector<analyzer::Token> rt;
	
		textwolf::UChar ch0;
		CharWindow scanner( src, srcsize, &m_punctuation_char);
		unsigned int wordlen=0;
		unsigned int pos = 0;

		for (; 0!=(ch0=scanner.chr(0)); wordlen=scanner.wordlen(),scanner.skip())
		{
			if (ch0 == '-')
			{
				textwolf::UChar ch1 = scanner.chr(1);
				if (isAlpha( ch1)) continue;
			}
			else if (ch0 == '.')
			{
				pos = scanner.itrpos();
				if (wordlen == 1)
				{
					// single characters followed by a dot.
#ifdef STRUS_LOWLEVEL_DEBUG
					std::cout << "ABBREV " << (int)__LINE__ << ":" << scanner.tostring() << std::endl;
#endif
					continue;
				}
				textwolf::UChar ch1 = scanner.chr(1);
				if (isDigit( ch1))
				{
					if (wordlen == 2 && isDigit(scanner.chr(2)))
					{
						// two digits followed by a dot
#ifdef STRUS_LOWLEVEL_DEBUG
						std::cout << "ABBREV " << scanner.tostring() << std::endl;
#endif
						continue;
					}
					// lookahead for number (dot part of number)
					wordlen = scanner.wordlen();
					scanner.skip();
					ch0 = scanner.chr(0);
					if (0==ch0)
					{
						// push punctuation for other case for previous character position (end of file)
						rt.push_back( analyzer::Token( pos/*ordpos*/, 0/*seg*/, pos, 1));
						break;
					}
					if (isDigit( ch0))
					{
						// dot in a number belongs to the number
#ifdef STRUS_LOWLEVEL_DEBUG
						std::cout << "ABBREV " << scanner.tostring() << std::endl;
#endif
						continue;
					}
					// push punctuation for other case for previous character position (lookahead)
#ifdef STRUS_LOWLEVEL_DEBUG
					std::cout << "PUNKT " << (int)__LINE__ << ":" << scanner.tostring() << std::endl;
#endif
					rt.push_back( analyzer::Token( pos/*ordpos*/, 0/*seg*/, pos, 1));
					continue;
				}
				else if (isLowercase( ch1))
				{
					// Check, if it is an abbreviation listed in the dictonary:
					char word[ CharWindow::NofPrevChar+1];
					word[ CharWindow::NofPrevChar] = 0;
					int wi = CharWindow::NofPrevChar;
					int ci = 1;
					textwolf::UChar ch = ch1|32;
					while (wi > 0 && ch >= 'a' && ch <= 'z')
					{
						word[ --wi] = (char)ch;
						ch = scanner.chr(++ci)|32;
					}
					if (wi > 0 && (ch == 0 || isSpace(ch) || !isPunctuation(ch)))
					{
						conotrie::CompactNodeTrie::NodeData val;
#ifdef STRUS_LOWLEVEL_DEBUG
						std::cout << "check abbreviation candidate '" << (word+wi) << "'" << std::endl;
#endif
						if (m_abbrevDict.get( word+wi, val))
						{
#ifdef STRUS_LOWLEVEL_DEBUG
							std::cout << "ABBREV " << (int)__LINE__ << ":" << scanner.tostring() << std::endl;
#endif
							continue;
						}
					}
	
					// Check some alternative heuristics:
					textwolf::UChar ch2 = scanner.chr(2);
					textwolf::UChar ch3 = scanner.chr(3);
					if (ch3)
					{
						// 3 subsequent consonants at the end
						// or 2 subsequent consonants in a word
						// with length <= 3
						if (isConsonant( ch1)
						&&  isConsonant( ch2) && ch2 != ch1)
						{
							if ((wordlen <= 3 
								&& (ch3 != 'i' && ch2 != 's' && ch1 != 't')
								&& (ch2 != 'h' && ch1 != 'r' && ch1 != 'n' && ch1 != 't')
								&& (ch2 != 'h' && ch1 != 'r')
								&& (ch3 != 'u' && ch2 != 'n' && ch1 != 's'))
							|| ((isConsonant( ch3) && ch3 != ch2 && wordlen <= 5)
								&& (ch3 != 'c' && ch2 != 'h' && ch1 != 't' && ch1 != 's')
								&& (ch3 != 'h' && ch2 != 'r' && ch1 != 't' && ch1 != 's')
							))
							{
#ifdef STRUS_LOWLEVEL_DEBUG
								std::cout << "ABBREV " << (int)__LINE__ << ":" << scanner.tostring() << std::endl;
#endif
								continue;
							}
						}
					}
					else
					{
						if (isConsonant( ch1) && isConsonant( ch2))
						{
#ifdef STRUS_LOWLEVEL_DEBUG
							std::cout << "ABBREV " << (int)__LINE__ << ":" << scanner.tostring() << std::endl;
#endif
							continue;
						}
					}
					if (ch1 == 'f' && ch2 == 'o' && ch3 == 'r')
					{
						// special case for "Prof."
#ifdef STRUS_LOWLEVEL_DEBUG
						std::cout << "ABBREV " << (int)__LINE__ << ":" << scanner.tostring() << std::endl;
#endif
						continue;
					}
					if (ch1 == 'c' && wordlen <= 3)
					{
						// c at the end of a small word
#ifdef STRUS_LOWLEVEL_DEBUG
						std::cout << "ABBREV " << (int)__LINE__ << ":" << scanner.tostring() << std::endl;
#endif
						continue;
					}
					if (ch1 == 'z' && ch2 != 't' && ch2 != 'n' && ch2 != 'r' && ch2 != 'l' && isConsonant( ch2))
					{
						// z after another consonant at the end a word
#ifdef STRUS_LOWLEVEL_DEBUG
						std::cout << "ABBREV " << (int)__LINE__ << ":" << scanner.tostring() << std::endl;
#endif
						continue;
					}
					if (ch1 == 'w' && isConsonant( ch2))
					{
						// w after another consonant at the end a word
#ifdef STRUS_LOWLEVEL_DEBUG
						std::cout << "ABBREV " << (int)__LINE__ << ":" << scanner.tostring() << std::endl;
#endif
						continue;
					}
					if (ch1 == 'v' && isConsonant( ch2))
					{
						// v after another consonant at the end a word
#ifdef STRUS_LOWLEVEL_DEBUG
						std::cout << "ABBREV " << (int)__LINE__ << ":" << scanner.tostring() << std::endl;
#endif
						continue;
					}
					if (ch1 == 'h' && ch2 != 'c' && isConsonant( ch2))
					{
						// h after a consonant other than 'c' at the end a word
#ifdef STRUS_LOWLEVEL_DEBUG
						std::cout << "ABBREV " << (int)__LINE__ << ":" << scanner.tostring() << std::endl;
#endif
						continue;
					}
					if (ch1 == 'k' && ch2 != 'c' && ch2 != 'l' && ch2 != 'n' && ch2 != 'r' && isConsonant( ch2))
					{
						// k after a consonant other than 'c','l','n' or 'r' at the end a word
#ifdef STRUS_LOWLEVEL_DEBUG
						std::cout << "ABBREV " << (int)__LINE__ << ":" << scanner.tostring() << std::endl;
#endif
						continue;
					}
				}
#ifdef STRUS_LOWLEVEL_DEBUG
				std::cout << "PUNKT " << (int)__LINE__ << ":" << scanner.tostring() << std::endl;
				std::size_t endpos = pos;
				std::size_t startpos = (endpos > 16)?(endpos-16):0;
				std::cout << "TOKEN AT " << std::string( src+startpos, endpos-startpos) << std::endl;
#endif
				rt.push_back( analyzer::Token( pos/*ordpos*/, 0/*seg*/, pos, 1));
			}
			else if (isPunctuation(ch0))
			{
				pos = scanner.itrpos();
#ifdef STRUS_LOWLEVEL_DEBUG
				std::cout << "PUNKT " << (int)__LINE__ << ":" << scanner.tostring() << std::endl;
				std::size_t endpos = pos;
				std::size_t startpos = (endpos > 16)?(endpos-16):0;
				std::cout << "TOKEN AT " << std::string( src+startpos, endpos-startpos) << std::endl;
#endif
				rt.push_back( analyzer::Token( pos/*ordpos*/, 0/*seg*/, pos, 1));
			}
		}
		return rt;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in 'punctuation' tokenizer: %s"), *m_errorhnd, std::vector<analyzer::Token>());
}

analyzer::FunctionView PunctuationTokenizerInstance_en::view() const
{
	try
	{
		return analyzer::FunctionView( "punctuation")
			( "language", "en")
			( "charlist", m_punctuation_charlist)
		;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in introspection: %s"), *m_errorhnd, analyzer::FunctionView());
}



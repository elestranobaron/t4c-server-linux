/******************************************************************************
Modify for vs2008 (06/05/2009)
/******************************************************************************/
#ifndef GAMEDEFS_H
#define GAMEDEFS_H


#define CL_BLUE_LIGHT2 	   RGB(  141, 227, 0) //RGB(  119, 180, 253 )

/******************************************************************************/
// Liste des couleurs
#define CL_BLACK           RGB(   0,   0,   0 )
#define CL_WHITE           RGB( 255, 255, 255 )
#define CL_GRAY            RGB( 128, 128, 128 )
#define CL_RED				   RGB( 255,   0,   0 )
#define CL_GREEN   		   RGB(   0, 255,   0 )
#define CL_BLUE			   RGB(   0,   0, 255 )
#define CL_YELLOW 		   RGB( 255, 220,   0 )
#define CL_ORANGE			   RGB( 255, 128,   0 )
#define CL_CYAN      	   RGB(   0, 192, 192 )
#define CL_PINK            RGB( 255, 130, 255 )
#define CL_PURPLE          RGB( 150,  25, 200 )
#define CL_BLUE_LIGHT  	   CL_BLUE_LIGHT2 //RGB(   0, 100, 255 )
#define CL_BLUE_2     	   RGB(   0, 155, 255 )
#define CL_GREEN_DARK      RGB(  50, 170,  10 )

#define CL_HEAL_DAMAGE_1   CL_YELLOW  //Yellow 
#define CL_HEAL_DAMAGE_2   RGB( 230,  55,  30 )  //Lightred
#define CL_HEAL_DAMAGE_3   RGB(  50, 255,  50)





/******************************************************************************/
// Liste des types de blocages pour les collisions, Il ne peut en avoir que 16 maximum 
#define __BLOCK_NONE				                             	 0
#define __BLOCK_ABSOLUTE			                         	 1
#define __BLOCK_CAN_FLY_OVER		                            2
#define __BLOCK_DEEP_WATER				                         3
#define __BLOCK_SHALLOW_WATER			                         4
#define __AREA_BUILDING					                         5
#define __SAFE_HAVEN					                            6
#define __INDOOR_SAFE_HAVEN			                      	 7
#define __AREA_FULL_PVP                                      8
#define __BLOCK_FORCE_FIELD                                  9
#define __FULL_PVP_CANNOT_REALLY_DIE_DROP_ORROB             10
#define __BLOCK_CAST_SPELL_WHEN_WALK_ON_THIS                11
#define __ARENAGAME_FULL_PVP                                12
#define __ARENAGAME_BT_FULL_PVP                             13
#define __ARENAGAME_RT_FULL_PVP                             14
#define __FULL_PVP_CANNOT_REALLY_DIE_DROP_ORROB_CAST_SPELL  15

/******************************************************************************/
// Type pour les apparences d'objets

const unsigned int __OBJGROUP_SHORT_SWORD	      		= 1;
const unsigned int __OBJGROUP_LONG_SWORD     			= 2;
const unsigned int __OBJGROUP_FLAIL				      	= 3;
const unsigned int __OBJGROUP_MORNING_STAR      		= 4;
const unsigned int __OBJGROUP_WARHAMMER		      		= 5;
const unsigned int __OBJGROUP_SPEAR    					= 6; 
const unsigned int __OBJGROUP_AXE		      			= 7;
const unsigned int __OBJGROUP_LEATHER_ARMOR	      		= 8;
const unsigned int __OBJGROUP_WELL_TALK					= 9;
const unsigned int __OBJGROUP_CHEST_TALK				= 10;
const unsigned int __OBJGROUP_LEATHER_HELM      		= 11;
const unsigned int __OBJGROUP_MAGE_SPELLBOOK		    = 12;
const unsigned int __OBJGROUP_PRIEST_ANKH    			= 13;
const unsigned int __OBJGROUP_SCROLL					= 14;
const unsigned int __OBJGROUP_WOODEN_DOOR_CLOSED		= 15;
const unsigned int __OBJGROUP_WOODEN_DOOR_OPENED		= 16;
const unsigned int __OBJGROUP_WOODEN_CHAIR				= 17;
const unsigned int __OBJGROUP_CHEST_I  		      		= 18;
const unsigned int __OBJGROUP_CHEST_OPEN_I				= 19;
const unsigned int __OBJGROUP_CHEST2_I					= 20;
const unsigned int __OBJGROUP_LONGUE_VUE				= 27;
const unsigned int __OBJGROUP_TORCH				      	= 28;
const unsigned int __OBJGROUP_IRON_KEY    				= 29;
const unsigned int __OBJGROUP_BLUE_KEY		      		= 30;
const unsigned int __OBJGROUP_WEIRD_KEY			      	= 31;
const unsigned int __OBJGROUP_GOLD_KEY 	   				= 32;
const unsigned int __OBJGROUP_KEY_HOLD	   	   			= 33;
const unsigned int __OBJGROUP_EMPTY_MUG      			= 34;
const unsigned int __OBJGROUP_BEER_MUG	      			= 35;
const unsigned int __OBJGROUP_BEER_MUG_TOO_FULL    		= 36;
const unsigned int __OBJGROUP_EMPTY_GLASS    			= 37;
const unsigned int __OBJGROUP_BEVERAGE			      	= 38;
const unsigned int __OBJGROUP_CUP_FULL					= 39;
const unsigned int __OBJGROUP_WOODEN_CUP     			= 40;
const unsigned int __OBJGROUP_CHEST				      	= 41;
const unsigned int __OBJGROUP_CHEST_OPEN				= 42;
const unsigned int __OBJGROUP_BASKET_CLOSE      		= 43;
const unsigned int __OBJGROUP_BASKET_OPEN			    = 44;
const unsigned int __OBJGROUP_RED_BOOK    				= 45;
const unsigned int __OBJGROUP_GREY_BOOK	      			= 46;
const unsigned int __OBJGROUP_BROWN_BOOK			    = 47;
const unsigned int __OBJGROUP_BACKPACK    				= 48;
const unsigned int __OBJGROUP_BANDAGES_PACK	      		= 49;
const unsigned int __OBJGROUP_BANDAGE_1      			= 50;
const unsigned int __OBJGROUP_BANDAGE_2		      		= 51;
const unsigned int __OBJGROUP_WOODEN_BOWL				= 52;
const unsigned int __OBJGROUP_WOODEN_VASE    			= 53;
const unsigned int __OBJGROUP_GOLD_BAR			      	= 54;
const unsigned int __OBJGROUP_GOLD_BAR_PACK				= 55;
const unsigned int __OBJGROUP_BRONZE_BAR     			= 56;
const unsigned int __OBJGROUP_BRONZE_BAR_PACK      		= 57;
const unsigned int __OBJGROUP_IRON_BAR		      		= 58;
const unsigned int __OBJGROUP_IRON_BAR_PACK	      		= 59;
const unsigned int __OBJGROUP_BROOM_ON_WALL	      		= 60;
const unsigned int __OBJGROUP_PIOCHE			      	= 61;
const unsigned int __OBJGROUP_BROOM_ON_FLOOR	      	= 62;
const unsigned int __OBJGROUP_HAMMER			      	= 63;
const unsigned int __OBJGROUP_PINCES			      	= 64;
const unsigned int __OBJGROUP_BALANCE			      	= 65;
const unsigned int __OBJGROUP_HOURGLASS		      		= 66;
const unsigned int __OBJGROUP_EPROUVETTE_6_PACKS      	= 67;
const unsigned int __OBJGROUP_EPROUVETTE_3_PACKS		= 68;
const unsigned int __OBJGROUP_EPROUVETTE_ALONE     		= 69;
const unsigned int __OBJGROUP_BUCHE_DE_BOIS	      		= 70;
const unsigned int __OBJGROUP_WOOD_PACK_CLASSED			= 71;
const unsigned int __OBJGROUP_WOOD_PACK_MIXED      		= 72;
const unsigned int __OBJGROUP_JAMBON	      			= 73;
const unsigned int __OBJGROUP_LONG_SAUSAGE	      		= 74;
const unsigned int __OBJGROUP_SAUSAGE_PACK	      		= 75;
const unsigned int __OBJGROUP_FAT_JAMBON		      	= 76;
const unsigned int __OBJGROUP_CHIKEN			      	= 77;
const unsigned int __OBJGROUP_BREAD				      	= 78;
const unsigned int __OBJGROUP_PACK_OF_PASTRIES	      	= 79;
const unsigned int __OBJGROUP_WATERBASKET		      	= 80;
const unsigned int __OBJGROUP_FISHING_POLE	      		= 81;
const unsigned int __OBJGROUP_DEAD_FISHES			    = 82;
const unsigned int __OBJGROUP_EMPTY_BASKET	      		= 83;
const unsigned int __OBJGROUP_ARROW				      	= 84;
const unsigned int __OBJGROUP_ARROW_PACK		      	= 85;
const unsigned int __OBJGROUP_BOW				      	= 86;
const unsigned int __OBJGROUP_CROSSBOW			      	= 87;
const unsigned int __OBJGROUP_BANJO				      	= 88;
const unsigned int __OBJGROUP_TAMBOUR				    = 89;
const unsigned int __OBJGROUP_MAP		      			= 90;
const unsigned int __OBJGROUP_SCIE				      	= 91;
const unsigned int __OBJGROUP_LONG_WHOOL_BALL      		= 92;
const unsigned int __OBJGROUP_WHOOL_BALL	      		= 93;
const unsigned int __OBJGROUP_GREEN_WHOOL_BALL	      	= 94;
const unsigned int __OBJGROUP_BLUE_WHOOL_BALL	      	= 95;
const unsigned int __OBJGROUP_SHOVEL		      		= 96;
const unsigned int __OBJGROUP_BLUE_POTION	      		= 97;
const unsigned int __OBJGROUP_YELLOW_POTION     		= 98;
const unsigned int __OBJGROUP_BLACK_POTION      		= 99;
const unsigned int __OBJGROUP_EMPTY_POTION      		= 100;
const unsigned int __OBJGROUP_PURPLE_POTION		      	= 101;
const unsigned int __OBJGROUP_BASKET_FULL_WATER    		= 102;
const unsigned int __OBJGROUP_BASKET_FULL_FRUIT    		= 103;
const unsigned int __OBJGROUP_WOODEN_ROUND_CHAIR      	= 104;
const unsigned int __OBJGROUP_WOODEN_CHAIR_2		    = 105;
const unsigned int __OBJGROUP_WOODEN_ROUND_CHAIR_2		= 106;
const unsigned int __OBJGROUP_CRATES		      		= 107;
const unsigned int __OBJGROUP_RECTANGLE_CRATES	      	= 108;
const unsigned int __OBJGROUP_CUBIQ_CRATES      		= 109;
const unsigned int __OBJGROUP_KITCHEN_BOWL      		= 110;
const unsigned int __OBJGROUP_KITCHEN_SPOON		      	= 111;
const unsigned int __OBJGROUP_KITCHEN_FORK	      		= 112;
const unsigned int __OBJGROUP_KITCHEN_KNIFE	      		= 113;
const unsigned int __OBJGROUP_ROULEAU_A_PATRE	      	= 114;
const unsigned int __OBJGROUP_CHANDELLE		      		= 115;
const unsigned int __OBJGROUP_CHANDELLE_SUR_PIED      	= 116;
const unsigned int __OBJGROUP_MIRROR	      			= 117;
const unsigned int __OBJGROUP_STAFF1			      	= 118;
const unsigned int __OBJGROUP_MACE                  	= 119;
const unsigned int __OBJGROUP_SPIKE_MACE		      	= 120;
const unsigned int __OBJGROUP_MAUL				      	= 121;
const unsigned int __OBJGROUP_DOUBLE_AXE		      	= 122;
const unsigned int __OBJGROUP_SINGLE_AXE		      	= 123;
const unsigned int __OBJGROUP_BASTARD_SWORD	      		= 124;
const unsigned int __OBJGROUP_SPEAR_AXE	      			= 125;
const unsigned int __OBJGROUP_SCROLL_OUVERT	      		= 126;
const unsigned int __OBJGROUP_SCROLL_BLUE	      		= 127;
const unsigned int __OBJGROUP_LIVRE_OUVERT	      		= 128;
const unsigned int __OBJGROUP_LIVRE_OUVERT_EPAIS      	= 129;
const unsigned int __OBJGROUP_WOODEN_DOOR_CLOSED_I		= 130;
const unsigned int __OBJGROUP_WOODEN_DOOR_OPENED_I		= 131;
const unsigned int __OBJGROUP_WOODEN_CHAIR_I	      	= 132; 
const unsigned int __OBJGROUP_WOODEN_CHAIR_2_I	      	= 133; 
const unsigned int __OBJGROUP_WOODEN_ROUND_CHAIR_2_I	= 134; 
const unsigned int __OBJGROUP_2_WOODEN_CHAIR		    = 135; //(shito 3)
const unsigned int __OBJGROUP_2_WOODEN_CHAIR_2			= 136; //(shtio 4)
const unsigned int __OBJGROUP_2_WOODEN_ROUND_CHAIR_2	= 137; //(shito 5)
const unsigned int __OBJGROUP_2_WOODEN_CHAIR_I			= 138; 
const unsigned int __OBJGROUP_2_WOODEN_CHAIR_2_I		= 139; 
const unsigned int __OBJGROUP_2_WOODEN_ROUND_CHAIR_2_I	= 140; 
const unsigned int __OBJGROUP_POUCH                     = 141;
const unsigned int __OBJGROUP_ROPE                      = 142;
const unsigned int __OBJGROUP_STUFFED_BASKET_1          = 143;
const unsigned int __OBJGROUP_STUFFED_BASKET_2          = 144;
const unsigned int __OBJGROUP_STUFFED_BASKET_3          = 145;
const unsigned int __OBJGROUP_FRUIT_1                   = 146;
const unsigned int __OBJGROUP_FRUIT_2                   = 147;
const unsigned int __OBJGROUP_FRUIT_3                   = 148;
const unsigned int __OBJGROUP_FRUIT_4                   = 149;
const unsigned int __OBJGROUP_FRUIT_5                   = 150;
const unsigned int __OBJGROUP_FRUIT_6                   = 151;
const unsigned int __OBJGROUP_FRUIT_7                   = 152;
const unsigned int __OBJGROUP_FRUIT_8                   = 153;
const unsigned int __OBJGROUP_FRUIT_9                   = 154;
const unsigned int __OBJGROUP_FRUIT_10                  = 155;
const unsigned int __OBJGROUP_FRUIT_11                  = 156;
const unsigned int __OBJGROUP_FRUIT_12                  = 157;
const unsigned int __OBJGROUP_FRUIT_13                  = 158;
const unsigned int __OBJGROUP_FRUIT_14                  = 159;
const unsigned int __OBJGROUP_COINS                     = 160;
const unsigned int __OBJGROUP_COINS_PILE                = 161;
const unsigned int __OBJGROUP_COINS_BIG_PILE            = 162;
const unsigned int __OBJGROUP_BATWINGS                  = 163;
const unsigned int __OBJGROUP_GEMS_PURPLE               = 164;
const unsigned int __OBJGROUP_GEMS_YELLOW               = 165;
const unsigned int __OBJGROUP_GEMS_BLUE                 = 166;
const unsigned int __OBJGROUP_GEMS_GREEN                = 167;
const unsigned int __OBJGROUP_GEMS_PACK_1               = 168;
const unsigned int __OBJGROUP_GEMS_PACK_2               = 169;
const unsigned int __OBJGROUP_GEMS_PACK_3               = 170;
const unsigned int __OBJGROUP_KOBOLDHAIR                = 171;      
const unsigned int __OBJGROUP_NECKLACE_1                = 172;
const unsigned int __OBJGROUP_NECKLACE_2                = 173;
const unsigned int __OBJGROUP_NECKLACE_3                = 174;
const unsigned int __OBJGROUP_ORCFEET                   = 175;
const unsigned int __OBJGROUP_RINGS_1                   = 176;
const unsigned int __OBJGROUP_RINGS_2                   = 177;
const unsigned int __OBJGROUP_RINGS_3                   = 178;  
const unsigned int __OBJGROUP_RINGS_4                   = 179;
const unsigned int __OBJGROUP_RINGS_5                   = 180;
const unsigned int __OBJGROUP_SKEL_BONE                 = 181;
const unsigned int __OBJGROUP_SPIDER_EYES               = 182;
const unsigned int __OBJGROUP_PADDED_GLOVE              = 183;
const unsigned int __OBJGROUP_PADDED_HELM               = 184;
const unsigned int __OBJGROUP_PADDED_ARMOR              = 185;
const unsigned int __OBJGROUP_PADDED_SLEEVES            = 186;
const unsigned int __OBJGROUP_PADDED_LEGGINGS           = 187;
const unsigned int __OBJGROUP_SCALE_ARMOR               = 188;
const unsigned int __OBJGROUP_LEVIER                    = 189;
const unsigned int __OBJGROUP_FIRECAMP                  = 190;
const unsigned int __OBJGROUP_CAMPBURNT                 = 201;
const unsigned int __OBJGROUP_GLINTING_SWORD            = 202;
const unsigned int __OBJGROUP_DESTINY_GEM               = 203;
const unsigned int __OBJGROUP_VOLCANO_ROCK              = 204;
const unsigned int __OBJGROUP_RING_HELM                 = 205;
const unsigned int __OBJGROUP_RING_ARMOR                = 206;
const unsigned int __OBJGROUP_RING_ARMOR_SLEEVES        = 207;
const unsigned int __OBJGROUP_RING_LEGGINGS             = 208;
const unsigned int __OBJGROUP_FEATHER                   = 209;
const unsigned int __OBJGROUP_ICESHARD                  = 210;
const unsigned int __OBJGROUP_LIFEGEM                   = 211;
const unsigned int __OBJGROUP_OAKTREELEAF               = 212;
const unsigned int __OBJGROUP_PADDED_BOOTS              = 213;
const unsigned int __OBJGROUP_TORCHE                    = 214;
const unsigned int __OBJGROUP_TORCHE_I                  = 215;
const unsigned int __OBJGROUP_BELT                      = 235;
const unsigned int __OBJGROUP_BLUEFLASK                 = 236;
const unsigned int __OBJGROUP_BRACELET                  = 237;
const unsigned int __OBJGROUP_CHEST2                    = 238;
const unsigned int __OBJGROUP_PINKLEAF                  = 239;
const unsigned int __OBJGROUP_POT_GREEN                 = 240;
const unsigned int __OBJGROUP_POT_RED                   = 241;
const unsigned int __OBJGROUP_SHIELD                    = 242;
const unsigned int __OBJGROUP_STONELIFE                 = 243;
const unsigned int __OBJGROUP_TORCH2                    = 244;
const unsigned int __OBJGROUP_TOOTH                     = 245;
const unsigned int __OBJGROUP_GODPOT_BLUE				= 246;
const unsigned int __OBJGROUP_GODPOT_GREEN				= 247;
const unsigned int __OBJGROUP_GODPOT_RED 				= 248;
const unsigned int __OBJGROUP_GODPOT_YELLOW				= 249;
const unsigned int __OBJGROUP_LARGEPOT_GREEN			= 250;
const unsigned int __OBJGROUP_LARGEPOT_RED  			= 251;
const unsigned int __OBJGROUP_POT_BLUE					= 252;
const unsigned int __OBJGROUP_POT_ORANGE				= 253;
const unsigned int __OBJGROUP_POT_TURQUOISE				= 254;
const unsigned int __OBJGROUP_POT_VIOLET				= 255;
const unsigned int __OBJGROUP_POT_YELLOW				= 256;
const unsigned int __OBJGROUP_GREENFLASK				= 257;
const unsigned int __OBJGROUP_REDFLASK					= 258;
const unsigned int __OBJGROUP_LEATHER_GLOVE			    = 259;
const unsigned int __OBJGROUP_LEATHER_BOOTS	  		    = 260;
const unsigned int __OBJGROUP_LEATHER_PANTS	  		    = 261;
const unsigned int __OBJGROUP_PLATE_ARMOR	      		= 262;
const unsigned int __OBJGROUP_PLATE_GLOVE	      		= 263;
const unsigned int __OBJGROUP_PLATE_ARMOR_W_SLEEVE		= 264;
const unsigned int __OBJGROUP_PLATE_BOOT	      		= 265;
const unsigned int __OBJGROUP_PLATE_LEGS	      		= 266;
const unsigned int __OBJGROUP_PLATE_HELM	      		= 267;
const unsigned int __OBJGROUP_CHAIN_LEGS				= 268;
const unsigned int __OBJGROUP_CHAIN_BODY				= 269;
const unsigned int __OBJGROUP_CHAIN_COIF				= 270;
const unsigned int __OBJGROUP_DARK_SWORD				= 271;
const unsigned int __OBJGROUP_ROMAN_SHIELD			    = 272;
const unsigned int __OBJGROUP_BAROSSA_SHIELD			= 273;
const unsigned int __OBJGROUP_DAGGER      		      	= 274;
const unsigned int __OBJGROUP_REAL_DARKSWORD			= 275;
const unsigned int __OBJGROUP_HORNED_HELMET			    = 276;
const unsigned int __OBJGROUP_BATTLE_SWORD			    = 277;
const unsigned int __OBJGROUP_NECROMANROBE			    = 278;
const unsigned int __OBJGROUP_GOLDENCROWN				= 279;
const unsigned int __OBJGROUP_GOLDEN_STAR				= 280;
const unsigned int __OBJGROUP_ELVEN_HAT				    = 281;
const unsigned int __OBJGROUP_ORC_SHIELD				= 282;
const unsigned int __OBJGROUP_STUDDEDARMOR			    = 283;
const unsigned int __OBJGROUP_LEG_CLOTH1				= 284;
const unsigned int __OBJGROUP_BODY_CLOTH1				= 285;
const unsigned int __OBJGROUP_STUDDEDLEG				= 286;
const unsigned int __OBJGROUP_REDCAPE					= 287;
const unsigned int __OBJGROUP_BLACKLEATHER_BOOT		    = 288;
const unsigned int __OBJGROUP_HEAD                      = 289;
const unsigned int __OBJGROUP_SUNDIAL_TALK              = 290;
const unsigned int __OBJGROUP_SIGN1_TALK                = 291;
const unsigned int __OBJGROUP_SIGN2_TALK                = 292;
const unsigned int __OBJGROUP_SIGN3_TALK                = 293;
const unsigned int __OBJGROUP_STAFF2			      	= 294;
const unsigned int __OBJGROUP_STAFF3			      	= 295;
const unsigned int __OBJGROUP_STAFF4			      	= 296;
const unsigned int __OBJGROUP_PORTAL   		      		= 297;
const unsigned int __OBJGROUP_VAULT_TALK                = 323;
const unsigned int __OBJGROUP_FOUGERE                   = 324;
const unsigned int __OBJGROUP_KRAANIAN_EGG              = 325;
const unsigned int __OBJGROUP_WOLF_PELT                 = 326;
const unsigned int __OBJGROUP_VAULT_TALK_I              = 327;
const unsigned int __OBJGROUP_HUMAN_FOOT                = 328;
const unsigned int __OBJGROUP_TROLLBADGE                = 329;
const unsigned int __OBJGROUP_SHOP_INN   		      	= 330;
const unsigned int __OBJGROUP_SHOP_POTION 	      		= 347;
const unsigned int __OBJGROUP_SHOP_PAWN   	      		= 360;
const unsigned int __OBJGROUP_SHOP_ARMOR 	         	= 373;
const unsigned int __OBJGROUP_SHOP_WEAPON 	      		= 390;
const unsigned int __OBJGROUP_SHOP_INN_I   		      	= 331;
const unsigned int __OBJGROUP_SHOP_POTION_I 	      	= 348;
const unsigned int __OBJGROUP_SHOP_PAWN_I   	      	= 361;
const unsigned int __OBJGROUP_SHOP_ARMOR_I 	         	= 374;
const unsigned int __OBJGROUP_SHOP_WEAPON_I 	      	= 391;
const unsigned int __OBJGROUP_LARGE_BOW                 = 421;
const unsigned int __OBJGROUP_QUIVER                    = 422;
const unsigned int __OBJGROUP_ARMOREDROBE			    = 423;
const unsigned int __OBJGROUP_MAGEROBE			        = 424;
const unsigned int __OBJGROUP_WHITEROBE			        = 425;
const unsigned int __OBJGROUP_CHEST_TROLL 	      		= 426;
const unsigned int __OBJGROUP_WHIRLPOOL                 = 427;
const unsigned int __OBJGROUP_OGRECLUB                  = 447;
const unsigned int __OBJGROUP_LARGE_BOW2                = 448;
const unsigned int __OBJGROUP_FANCY_SHORT_BOW           = 449;
const unsigned int __OBJGROUP_FANCY_LONG_BOW            = 450;
const unsigned int __OBJGROUP_BLUE_QUIVER               = 451;
const unsigned int __OBJGROUP_YELLOW_QUIVER             = 452;
const unsigned int __OBJGROUP_GREEN_QUIVER              = 453;
const unsigned int __OBJGROUP_RED_QUIVER                = 454;
const unsigned int __OBJGROUP_BLACK_QUIVER              = 455;
const unsigned int __OBJGROUP_DOOR_TALK                 = 456;
const unsigned int __OBJGROUP_DOOR_TALK_I               = 457;
const unsigned int __OBJGROUP_VAULT                     = 458;
const unsigned int __OBJGROUP_VAULT_I                   = 459;
const unsigned int __OBJGROUP_CENTAUR_SHIELD1           = 460;
const unsigned int __OBJGROUP_CENTAUR_SHIELD2           = 461;
const unsigned int __OBJGROUP_SHAMAN_HELM               = 462;
const unsigned int __OBJGROUP_SKAVEN_CLUB               = 463;
const unsigned int __OBJGROUP_SKAVEN_KNIFE              = 464;
const unsigned int __OBJGROUP_SKAVEN_SHIELD1            = 465;
const unsigned int __OBJGROUP_SKAVEN_SHIELD2            = 466;
const unsigned int __OBJGROUP_SKAVEN_SHIELD3            = 467;
const unsigned int __OBJGROUP_SKELETON_AXE              = 468;
const unsigned int __OBJGROUP_SKELETON_HELM             = 469;
const unsigned int __OBJGROUP_SKELETON_SHIELD           = 470;
const unsigned int __OBJGROUP_SKELETON_SWORD            = 471;
const unsigned int __OBJGROUP_SERAPH_BLACK_WINGS        = 472;
const unsigned int __OBJGROUP_SERAPH_WHITE_WINGS        = 473;
const unsigned int __OBJGROUP_DARK_GEM                  = 474;
const unsigned int __OBJGROUP_PURPLE_GEM                = 475;
const unsigned int __OBJGROUP_RED_GEM                   = 476;
const unsigned int __OBJGROUP_WHIRLPOOL2                = 477;
const unsigned int __OBJGROUP_COFFIN_TALK               = 497;
const unsigned int __OBJGROUP_HALBERD                   = 498;
const unsigned int __OBJGROUP_SMALL_PURPLE_GEM          = 499;
const unsigned int __OBJGROUP_SMALL_RED_GEM             = 500;
const unsigned int __OBJGROUP_CAULDRON                  = 501;
const unsigned int __OBJGROUP_CAULDRON_GROUND           = 522;
const unsigned int __OBJGROUP_SKAVEN_CORPSE1            = 543;
const unsigned int __OBJGROUP_SKAVEN_CORPSE2            = 544;
const unsigned int __OBJGROUP_SKAVEN_CORPSE3            = 545;
const unsigned int __OBJGROUP_SKAVEN_CORPSE4            = 546;
const unsigned int __OBJGROUP_SKAVEN_CORPSE_I1          = 547;
const unsigned int __OBJGROUP_SKAVEN_CORPSE_I2          = 548;
const unsigned int __OBJGROUP_SKAVEN_CORPSE_I3          = 549;
const unsigned int __OBJGROUP_SKAVEN_CORPSE_I4          = 550;
const unsigned int __OBJGROUP_RIB_TALK                  = 551;
const unsigned int __OBJGROUP_DS_DARK_WINGS             = 578;
const unsigned int __OBJGROUP_BROWNROBE                 = 595;
const unsigned int __OBJGROUP_WOODEN_DOOR2_CLOSED		= 769; //door2 1
const unsigned int __OBJGROUP_WOODEN_DOOR2_OPENED		= 770; //door2 11
const unsigned int __OBJGROUP_WOODEN_DOOR2_CLOSED_I		= 771;
const unsigned int __OBJGROUP_WOODEN_DOOR2_OPENED_I		= 772;
const unsigned int __OBJGROUP_WOODEN_DOOR3_CLOSED		= 791; //door3 1
const unsigned int __OBJGROUP_WOODEN_DOOR3_OPENED		= 792; //door3 11
const unsigned int __OBJGROUP_WOODEN_DOOR3_CLOSED_I		= 793;
const unsigned int __OBJGROUP_WOODEN_DOOR3_OPENED_I		= 794;
const unsigned int __OBJGROUP_WOODEN_DOOR4_CLOSED		= 804; //door4 1
const unsigned int __OBJGROUP_WOODEN_DOOR4_OPENED		= 805; //door4 11
const unsigned int __OBJGROUP_WOODEN_DOOR4_CLOSED_I		= 806;
const unsigned int __OBJGROUP_WOODEN_DOOR5_CLOSED		= 817; //door5 1
const unsigned int __OBJGROUP_WOODEN_DOOR5_OPENED		= 818; //door5 11
const unsigned int __OBJGROUP_WOODEN_DOOR5_CLOSED_I		= 819;
const unsigned int __OBJGROUP_WOODEN_DOOR5_OPENED_I		= 820;
const unsigned int __OBJGROUP_WOODEN_DOOR6_CLOSED		= 830; //door6 1
const unsigned int __OBJGROUP_WOODEN_DOOR6_OPENED		= 831; //door6 11
const unsigned int __OBJGROUP_WOODEN_DOOR6_CLOSED_I		= 832;
const unsigned int __OBJGROUP_WOODEN_DOOR6_OPENED_I		= 833;
const unsigned int __OBJGROUP_WOODEN_DOOR7_CLOSED		= 843; //door7 1
const unsigned int __OBJGROUP_WOODEN_DOOR7_OPENED		= 844; //door7 11
const unsigned int __OBJGROUP_WOODEN_DOOR7_CLOSED_I		= 845;
const unsigned int __OBJGROUP_WOODEN_DOOR7_OPENED_I		= 846;
const unsigned int __OBJGROUP_WOODEN_DOOR8_CLOSED		= 856; //door8 1
const unsigned int __OBJGROUP_WOODEN_DOOR8_OPENED		= 857; //door8 11
const unsigned int __OBJGROUP_WOODEN_DOOR8_CLOSED_I		= 858;
const unsigned int __OBJGROUP_WOODEN_DOOR8_OPENED_I		= 859;
const unsigned int __OBJGROUP_AMNET_WINGS  	      = 888;
const unsigned int __OBJGROUP_CAULDRON_CONTAINER	= 897;
const unsigned int __OBJGROUP_CAULDRON_CONTAINER_E = 917;
const unsigned int __OBJGROUP_WHITELICHROBE	      = 925;

static const int GET									= 5; 
static const int USE									= 4;	  
static const int TALK									= 3;
static const int ATTACK									= 2;
static const int NONE									= 0;
static const int USE_ONSITE								= 6;

/******************************************************************************/
//Liste des boosts
#define BOOST_STRONG_BLOW								0x0F000001
#define BOOST_UNDEAD_ATTACK_SKILL						0x0F000002
#define BOOST_UNDEAD_DAMAGE_ROLL						0x0F000003
#define BOOST_DISEASE									0x0F000004
#define BOOST_EQUIP_WEAPON1_DAM							0x0F000005
#define BOOST_EQUIP_WEAPON1_ATTACK						0x0F000006
#define BOOST_EQUIP_WEAPON2_DAM							0x0F000005
#define BOOST_EQUIP_WEAPON2_ATTACK						0x0F000006
#define BOOST_EQUIP_ARMOR_BODY_DODGE					0x0F000007
#define BOOST_EQUIP_ARMOR_FEET_DODGE					0x0F000008
#define BOOST_EQUIP_ARMOR_GLOVES_DODGE					0x0F000009
#define BOOST_EQUIP_ARMOR_HELM_DODGE					0x0F00000A
#define BOOST_EQUIP_ARMOR_LEGS_DODGE					0x0F00000B
#define BOOST_EQUIP_ARMOR_SLEEVES_DODGE					0x0F00000C
#define BOOST_SKILL_IMMOBILIZATION_DODGE			    0x0F00000D

/******************************************************************************/
//Liste des clans pour les créatures
#define NB_CLANS										48

#define __CLAN_NONE										0
#define __CLAN_LIGHTHAVEN								1
#define __CLAN_WINDHOWL									2
#define __CLAN_SILVERSKY								3
#define __CLAN_HAWKS_NEST								4
#define __CLAN_SUNSHADOW								5
#define __CLAN_VOLKARA									6
#define __CLAN_BLUEROCK									7
#define __CLAN_ASMORDIA									8
#define __CLAN_DUNGEON_DWELLER							9
#define __CLAN_DUNGEON_OF_BALORK						10
#define __CLAN_BRIGAND_MOUNTAIN							11
#define __CLAN_DRUID_CAMP								12
#define __CLAN_ANIMALS									13
#define __CLAN_UNDEAD_FROM_GRAVEYARD					14
#define __CLAN_ORC_BAND									15
#define __CLAN_INSECT_AND_ANIMALS						16
#define __CLAN_GOBLIN									17
#define __CLAN_KRAANIAN									18
#define __CLAN_ORKANIS									19
#define __CLAN_JARKO									20
#define __CLAN_ANCIENT_TEMPLE							21
#define __CLAN_ORC_DESERTER								22
#define __CLAN_BANE_GUILD               				23
#define __CLAN_RAVEN_MONSTER            				24
#define __CLAN_TOWN_NPC                 				25
#define __CLAN_LAGBEAST                 				26
#define __CLAN_PEACE                    				27
#define __CLAN_MOON_TUG_TRIBE           				28
#define __CLAN_KAHP_LETH_TRIBE          				29
#define __CLAN_STONEHEIM_CENTAURS       				30
#define __CLAN_STONEHEIM_SPIDERS        				31
#define __CLAN_STONEHEIM_WASPS          				32
#define __CLAN_STONEHEIM_WOLVES         				33
#define __CLAN_STONEHEIM_UNDEAD         				34
#define __CLAN_STONEHEIM_MISC           				35
#define __CLAN_MORDENTHAL_DUNGEON       				36
#define __CLAN_MORDENTHAL_CAVE          				37 
#define __CLAN_NETHER_ISLES             				38
#define __CLAN_PURIFIER                 				39
#define __CLAN_CREEPING_DEATH           				40
#define __CLAN_SUNKEN_WOODS             				41
#define __CLAN_CENTAUR                  				42
#define __CLAN_CENTAUR_CAVE_MONSTERS    				43
#define __CLAN_MAKRSH_PTANGH            				44
#define __CLAN_PSYKOWASP                				45
#define __CLAN_ATHENA_KEEP              				46
#define __CLAN_MADRIGAN                 				47

/******************************************************************************/
//Liste des clans pour les pnjs
#define __CLAN_CHURCH_OF_ARTHERK						0xA001

/******************************************************************************/
// Here is listed what creatures can do..
#define nothing											0
#define fighting										1
#define talking											2
#define wandering										3
//#define put_to_death									4
#define flee											5

/******************************************************************************/
// Liste des effets
#define EFFECT_STUN_BLOW								1
#define EFFECT_WAR_CRY									2
#define EFFECT_REMOVE_TRAP_DETECTED						3
#define EFFECT_BASH										4
#define EFFECT_BERSERK									5
#define EFFECT_REMOVE_LIGHT								6
#define EFFECT_REMOVE_INVISIBLE_HAND					7
#define EFFECT_STRONG_BLOW								8
#define EFFECT_STRONG_BLOW_QUERY_ATTACK					9
#define EFFECT_RESIST_CONTROL_ANIMAL					10
#define EFFECT_SKIN_OF_WOOD								11
#define EFFECT_REMOVE_FLY								12
#define EFFECT_DISEASE_TOUCH							13
#define EFFECT_REMOVE_WALK_ON_WATER						14
#define EFFECT_FLAG_ADDING_SPELL            			15
#define EFFECT_BASH_STUN                    			16
#define EFFECT_FIRST_AID_EXHAUST            			17
#define EFFECT_HIDE_SKILL                   			18
#define EFFECT_POWERFULLBLOW_COOLDOWN					19
#define EFFECT_IMMOBILIZATION						    20
#define EFFECT_IMMOBILIZATION_EXHAUST					21
#define EFFECT_IMMOBILIZATION_BR    					22
#define EFFECT_PRIMALSCREAM   						    23
#define EFFECT_PRIMALSCREAM_EXHAUST                     24


/******************************************************************************/
// this file lists all possible events
#define __EVENT_OBJECT_MOVED							1
#define __EVENT_OBJECT_REMOVED							11
#define __EVENT_OBJECT_CHANGED							12
#define __EVENT_OBJECT_APPEARED_LIST					16
#define __EVENT_SHOUT									27
#define __EVENT_SOMETHING_DIED							10000
#define __EVENT_ATTACK									10001
#define __EVENT_MISS									10002
#define __EVENT_SKILL_USED								10003
//#define __EVENT_OBJECT_APPEARED						10004
#define __EVENT_SPELL_EFFECT                			64

/******************************************************************************/
// Flags which tell where is the object equipped
#define __FLAG_CANNOT_DISPELL							12 // Tells that no "DISPELL MAGIC" spell can destroy the effect of this object
#define __FLAG_BLOCKING            						14 // Tells that the object is "blocking"
#define __FLAG_DOOR_LOCKED         						15 // Tells that a door is locked (or a safe)
#define __FLAG_OPENED_DOOR         						16 // Tells that a door is opened
#define __FLAG_CHARGES			   						27 // Flag that hold the number of charges on an item
#define __FLAG_TRAP_DISABLED	   						28 // None zero if trap in object was disabled.
#define __FLAG_ICY_SABER		   						29 //	Holds the icy sabre's strength.
#define __FLAG_DONT_RESPAWN		   						30 // If non-zero, forbids a container object from respawning.
#define __FLAG_GOLD                						31 // Quantity of gold on a Gold type of object.
#define __FLAG_USE_RANGE           						32 // Range from which the object can be used.
#define __FLAG_BLOCKING_SIZE                       33 // Tells that the object is "blocking"
#define __FLAG_BLOCKING_FLIP                       34 // Tells that the object is "blocking"

// Flag de Professions
#define __FLAG_PROF_APOTICAIRE  50
#define __FLAG_PROF_BIJOUTIER   51
#define __FLAG_PROF_COUTURIER   52
#define __FLAG_PROF_ARMURIER    53
#define __FLAG_PROF_FORGERON    54
#define __FLAG_PROF_EBENISTE    55

/******************************************************************************/
// Flags for creatures
const unsigned int __FLAG_APPEARANCE					= 10000;
const unsigned int __FLAG_CREATURE_TYPE					= 10001;

/******************************************************************************/
// Defines the different monster appearances
#define __MOBAPPEAR_PUPPET								10011
#define __MOBAPPEAR_GOBLIN								20001
#define __MOBAPPEAR_BAT									20002
#define __MOBAPPEAR_RAT									20003
#define __MOBAPPEAR_KOBOLD								20004
#define __MOBAPPEAR_OOZE								20005
#define __MOBAPPEAR_BRIGAND								20006
#define __MOBAPPEAR_SPIDER								20007
#define __MOBAPPEAR_ORC									20008
#define __MOBAPPEAR_ZOMBIE								20009
#define __MOBAPPEAR_GREEN_TROLL							20010
#define __MOBAPPEAR_MUMMY								20011
#define __MOBAPPEAR_SKELETON							20012
#define __MOBAPPEAR_DEMON								20013
#define __MOBAPPEAR_MINOTAUR							20014
#define __MOBAPPEAR_BEHOLDER							20015
#define __MOBAPPEAR_SMALL_WORM							20016
#define __MOBAPPEAR_BIG_WORM							20017
#define __MOBAPPEAR_TREE_ENT            				20018
#define __MOBAPPEAR_SNAKE               				20019
#define __MOBAPPEAR_UNICORN             				20020
#define __MOBAPPEAR_CENTAUR             				20021
#define __MOBAPPEAR_HORSE               				20022
#define __MOBAPPEAR_PEGASUS             				20023
#define __MOBAPPEAR_SCORPION            				20024
#define __MOBAPPEAR_KRAANIAN            				20025
#define __MOBAPPEAR_ATROCITY            				20026
#define __MOBAPPEAR_NIGHTMARE           				20027
#define __MOBAPPEAR_DRAGON              				20028
#define __MOBAPPEAR_WASP                				20029
#define __MOBAPPEAR_DROMADARY           				20030
#define __MOBAPPEAR_PIG                 				20031
#define __MOBAPPEAR_ORI                 				20032
#define __MOBAPPEAR_TARANTULA           				20033
#define __MOBAPPEAR_KRAANIANFLYING      				20034
#define __MOBAPPEAR_KRAANIANMILIPEDE    				20035
#define __MOBAPPEAR_AGMORKIAN           				20036
#define __MOBAPPEAR_KRAANIANTANK        				20037
#define __MOBAPPEAR_TAUNTING            				20038
#define __MOBAPPEAR_HUMAN_MAGE          				20039
#define __MOBAPPEAR_ATROCITYBOSS        				20040
#define __MOBAPPEAR_RED_GOBLINBOSS      				20041
#define __MOBAPPEAR_HUMAN_THIEF         				20042
#define __MOBAPPEAR_HUMAN_SWORDMAN      				20043
#define __MOBAPPEAR_HUMAN_PRIEST        				20044
#define __MOBAPPEAR_WOLF                				20045
#define __MOBAPPEAR_DARKWOLF     						20046
#define __MOBAPPEAR_SKAVEN_PEON         				20047   
#define __MOBAPPEAR_SKAVEN_SHAMAN       				20048   
#define __MOBAPPEAR_SKAVEN_SKAVENGER    				20049   
#define __MOBAPPEAR_SKAVEN_WARRIOR      				20050   
#define __MOBAPPEAR_CENTAUR_WARRIOR     				20051   
#define __MOBAPPEAR_CENTAUR_ARCHER      				20052   
#define __MOBAPPEAR_CENTAUR_SHAMAN      				20053   
#define __MOBAPPEAR_CENTAUR_KING        				20054   
#define __MOBAPPEAR_SKELETON_SERVANT_1  				20055
#define __MOBAPPEAR_SKELETON_SERVANT_2  				20056
#define __MOBAPPEAR_SKELETON_KING       				20057
#define __MOBAPPEAR_LICH                				20058
#define __MOBAPPEAR_SKAVEN_PEON2        				20059   
#define __MOBAPPEAR_SKAVEN_SHAMAN2      				20060   
#define __MOBAPPEAR_SKAVEN_SKAVENGER2   				20061   
#define __MOBAPPEAR_SKAVEN_WARRIOR2     				20062
#define __MOBAPPEAR_SKELETON_CENTAUR    				20063
#define __MOBAPPEAR_DRACONIS_LEATHER					20067
#define __MOBAPPEAR_FROZEN_KRAANIANTANK					20105
#define __MOBAPPEAR_DSTREE          	            20070
#define __MOBAPPEAR_OGRESSE          	            20126

#define __MOBAPPEAR_INVISIBLE_THIEF						21042
#define __MOBAPPEAR_INVISIBLE_PRIEST					21044
#define __MOBAPPEAR_CORPSE_INVISIBLE_PRIEST				15044
#define __MOBAPPEAR_CORPSE_INVISIBLE_THIEF				15004

#define __MOBAPPEAR_CORPSE_PUPPET						15011
#define __MOBAPPEAR_CORPSE_GOBLIN		        		25001
#define __MOBAPPEAR_CORPSE_BAT			        		25002
#define __MOBAPPEAR_CORPSE_RAT			        		25003
#define __MOBAPPEAR_CORPSE_KOBOLD		        		25004
#define __MOBAPPEAR_CORPSE_OOZE			        		25005
#define __MOBAPPEAR_CORPSE_BRIGAND		        		25006
#define __MOBAPPEAR_CORPSE_SPIDER		        		25007
#define __MOBAPPEAR_CORPSE_ORC			        		25008
#define __MOBAPPEAR_CORPSE_ZOMBIE		        		25009
#define __MOBAPPEAR_CORPSE_GREEN_TROLL	        		25010
#define __MOBAPPEAR_CORPSE_MUMMY		        		25011
#define __MOBAPPEAR_CORPSE_SKELETON		        		25012
#define __MOBAPPEAR_CORPSE_DEMON		        		25013
#define __MOBAPPEAR_CORPSE_MINOTAUR		        		25014
#define __MOBAPPEAR_CORPSE_BEHOLDER		        		25015
#define __MOBAPPEAR_CORPSE_SMALL_WORM	        		25016
#define __MOBAPPEAR_CORPSE_BIG_WORM		        		25017
#define __MOBAPPEAR_CORPSE_TREE_ENT             		25018
#define __MOBAPPEAR_CORPSE_SNAKE                		25019
#define __MOBAPPEAR_CORPSE_UNICORN              		25020
#define __MOBAPPEAR_CORPSE_CENTAUR              		25021
#define __MOBAPPEAR_CORPSE_HORSE                		25022
#define __MOBAPPEAR_CORPSE_PEGASUS              		25023
#define __MOBAPPEAR_CORPSE_SCORPION             		25024
#define __MOBAPPEAR_CORPSE_KRAANIAN             		25025
#define __MOBAPPEAR_CORPSE_ATROCITY             		25026
#define __MOBAPPEAR_CORPSE_NIGHTMARE            		25027
#define __MOBAPPEAR_CORPSE_DRAGON               		25028
#define __MOBAPPEAR_CORPSE_WASP                 		25029
#define __MOBAPPEAR_CORPSE_DROMADARY            		25030
#define __MOBAPPEAR_CORPSE_PIG                  		25031
#define __MOBAPPEAR_CORPSE_ORI                  		25032
#define __MOBAPPEAR_CORPSE_TARANTULA            		25033
#define __MOBAPPEAR_CORPSE_KRAANIANFLYING       		25034
#define __MOBAPPEAR_CORPSE_KRAANIANMILIPEDE     		25035
#define __MOBAPPEAR_CORPSE_AGMORKIAN            		25036
#define __MOBAPPEAR_CORPSE_KRAANIANTANK         		25037
#define __MOBAPPEAR_CORPSE_TAUNTING             		25038
#define __MOBAPPEAR_CORPSE_HUMAN_MAGE           		25039
#define __MOBAPPEAR_CORPSE_ATROCITYBOSS         		25040
#define __MOBAPPEAR_CORPSE_RED_GOBLINBOSS       		25041
#define __MOBAPPEAR_CORPSE_HUMAN_THIEF          		25042
#define __MOBAPPEAR_CORPSE_HUMAN_SWORDMAN       		25043
#define __MOBAPPEAR_CORPSE_HUMAN_PRIEST         		25044
#define __MOBAPPEAR_CORPSE_WOLF         	    		25045
#define __MOBAPPEAR_CORPSE_DARKWOLF     				25046
#define __MOBAPPEAR_CORPSE_SKAVEN_PEON          		25047   
#define __MOBAPPEAR_CORPSE_SKAVEN_SHAMAN        		25048   
#define __MOBAPPEAR_CORPSE_SKAVEN_SKAVENGER     		25049   
#define __MOBAPPEAR_CORPSE_SKAVEN_WARRIOR       		25050   
#define __MOBAPPEAR_CORPSE_CENTAUR_WARRIOR      		25051   
#define __MOBAPPEAR_CORPSE_CENTAUR_ARCHER       		25052   
#define __MOBAPPEAR_CORPSE_CENTAUR_SHAMAN       		25053   
#define __MOBAPPEAR_CORPSE_CENTAUR_KING         		25054   
#define __MOBAPPEAR_CORPSE_SKELETON_SERVANT_1   		25055
#define __MOBAPPEAR_CORPSE_SKELETON_SERVANT_2   		25056
#define __MOBAPPEAR_CORPSE_SKELETON_KING        		25057
#define __MOBAPPEAR_CORPSE_LICH                 		25058
#define __MOBAPPEAR_CORPSE_SKAVEN_PEON2         		25059   
#define __MOBAPPEAR_CORPSE_SKAVEN_SHAMAN2       		25060   
#define __MOBAPPEAR_CORPSE_SKAVEN_SKAVENGER2    		25061   
#define __MOBAPPEAR_CORPSE_SKAVEN_WARRIOR2      		25062
#define __MOBAPPEAR_CORPSE_SKELETON_CENTAUR     		25063
#define __MOBAPPEAR_CORPSE_DRACONIS_LEATHER				25067
#define __MOBAPPEAR_CORPSE_FROZEN_KRAANIANTANK			20105
#define __MOBAPPEAR_CORPSE_DSTREE          	            25070
#define __MOBAPPEAR_CORPSE_OGRESSE           	        25126

/******************************************************************************/
//Liste des races
#define __PLAYER_HUMAN_SWORDMAN							10001
#define __PLAYER_HUMAN_MAGE								10002
#define __PLAYER_HUMAN_PRIEST							10003
#define __PLAYER_HUMAN_THIEF							10004
#define __NPC_PLAYER_SWORDMAN       					10001
#define __NPC_PLAYER_MAGE           					10002
#define __NPC_PLAYER_PRIEST         					10003
#define __NPC_PLAYER_THIEF          					10004
#define __NPC_HUMAN_PEASANT								10005
#define __NPC_HUMAN_GUARD								10006
#define __NPC_HUMAN_MAGE								10007
#define __NPC_HUMAN_PAYSANNE							10008
#define __NPC_HUMAN_PRIEST								10009
#define __NPC_HUMAN_WARRIOR								10010
#define __NPC_HUMAN_THIEF								10010
#define __NPC_TROLL                 					20010
#define __NPC_ORC                   					20008
#define __NPC_DEMON                 					20013
#define __NPC_PIG                   					20031
#define __PLAYER_PUPPET             					10011
#define __PLAYER_FEMALE_PUPPET      					10012
#define __NPC_PUPPET                					10011
#define __NPC_FEMALE_PUPPET         					10012
#define __NPC_DRAGON                					20028
#define __NPC_GOBLIN                					20001

#define __NPC_CORPSE_PUPPET         					15011
#define __PLAYER_PUPPET_CORPSE      					15011
#define __PLAYER_FEMALE_PUPPET_CORPSE 					15012
#define __NPC_CORPSE_FEMALE_PUPPET      				15012
#define __PLAYER_HUMAN_SWORDMAN_CORPSE					15001
#define __PLAYER_HUMAN_MAGE_CORPSE						15002
#define __PLAYER_HUMAN_PRIEST_CORPSE					15003
#define __PLAYER_HUMAN_THIEF_CORPSE						15004
#define __NPC_CORPSE_PLAYER_SWORDMAN					15001
#define __NPC_CORPSE_PLAYER_MAGE						15002
#define __NPC_CORPSE_PLAYER_PRIEST         				15003
#define __NPC_CORPSE_PLAYER_THIEF          				15004
#define __NPC_CORPSE_PLAYER_MAGE        				15002
#define __NPC_CORPSE_HUMAN_PEASANT						15005
#define __NPC_CORPSE_HUMAN_GUARD						15006
#define __NPC_CORPSE_HUMAN_MAGE							15007
#define __NPC_CORPSE_HUMAN_PAYSANNE						15008
#define __NPC_CORPSE_HUMAN_PRIEST						15009
#define __NPC_CORPSE_HUMAN_WARRIOR						15010
#define __NPC_CORPSE_HUMAN_THIEF						15010
#define __NPC_CORPSE_ORC                				25008
#define __NPC_CORPSE_TROLL              				25010
#define __NPC_CORPSE_DEMON              				25013
#define __NPC_CORPSE_PIG                				25031
#define __NPC_CORPSE_DRAGON             				25028
#define __NPC_CORPSE_GOBLIN             				25001

/******************************************************************************/
//Defines the standard, known spell effect structures.
#define HEALTH_EFFECT									1       // done
#define ATTR_BOOST_EFFECT       						2       // done
#define FLAG_ADDING_EFFECT      						3       // done
#define RESISTANCE_EFFECT       						5
#define CONJURE_EFFECT          						6       // done
#define RECALL_EFFECT           						7       // done
#define DISPELL_EFFECT          						13
#define SPELL_HOOK_EFFECT       						9       // done
#define DRAIN_LIFE_EFFECT       						10
#define RECALLDEATHPOS_EFFECT   						11
#define VAPORIZE_UNIT_EFFECT    						12
#define EXHAUST_EFFECT          						14
#define INVISIBILITY_EFFECT     						15
#define DETECT_INVISIBILITY_EFFECT  				16
#define DETECT_HIDDEN_EFFECT        				17
#define ITEMACTION_EFFECT								18
#define SYSMSG_EFFECT									19
#define SETSTAT_EFFECT									20
#define PERSISTENT_SPELL_HOOK_EFFECT 				21
#define GIVEGOLDXP_EFFECT								22
#define REVOKE_UNIT_EFFECT								23
#define INVISIBILITY2_EFFECT                    24
#define MOVEOUT_EFFECT                          25
#define TAKEGOLD_EFFECT                         26
#define DRAIN_MANA_EFFECT       						27
#define FRIENDLY_EFFECT       						28
#define SPELL_POSITION_EFFECT    					29


#endif // GAMEDEFS_H
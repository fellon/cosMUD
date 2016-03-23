/******************************************************
            Desolation of the Dragon MUD II
      (C) 1997-2002  Jesse DeFer and Heath Leach
          http://www.dotd.com  dotd@dotd.com
 ******************************************************/

#define ASSIGN_GSN(gsn, skill)					\
do								\
{								\
    if ( ((gsn) = skill_lookup((skill))) == -1 )		\
        boot_log( "ASSIGN_GSN: Skill %s not found.\n",	\
		(skill) );					\
} while(0)

extern	int	gsn_top_sn;
extern	int	gsn_first_lore;
extern	int	gsn_first_skill;
extern	int	gsn_first_spell;
extern	int	gsn_first_tongue;
extern	int	gsn_first_weapon;

#define GSN_LAST_SPELL	(gsn_first_skill-1)
#define GSN_LAST_SKILL	(gsn_first_weapon-1)
#define GSN_LAST_WEAPON	(gsn_first_tongue-1)
#define GSN_LAST_TONGUE	(gsn_first_lore-1)
#define GSN_LAST_LORE	(gsn_top_sn-1)

/* --- */
extern int gsn_avoid_back_attack;
extern int gsn_detect_invis;
extern int gsn_enlarge;
extern int gsn_detect_evil;
extern int gsn_detect_magic;
extern int gsn_sense_life;
extern int gsn_sanctuary;
extern int gsn_poly;
extern int gsn_protection_from_evil;
extern int gsn_infravision;
extern int gsn_charm_person;
extern int gsn_weakness;
extern int gsn_strength;
extern int gsn_armor;
extern int gsn_detect_poison;
extern int gsn_bless;
extern int gsn_fly;
extern int gsn_water_breath;
extern int gsn_fireshield;
extern int gsn_faerie_fire;
extern int gsn_minor_track;
extern int gsn_major_track;
extern int gsn_web;
extern int gsn_silence;
extern int gsn_tree_travel;
extern int gsn_haste;
extern int gsn_slow;
extern int gsn_barkskin;
extern int gsn_aid;
extern int gsn_true_sight;
extern int gsn_invis_to_animals;
extern int gsn_dragon_ride;
extern int gsn_darkness;
extern int gsn_minor_invulnerability;
extern int gsn_major_invulnerability;
extern int gsn_protection_from_energy_drain;
extern int gsn_wizardeye;
extern int gsn_protection_from_breath;
extern int gsn_protection_from_fire_breath;
extern int gsn_protection_from_frost_breath;
extern int gsn_protection_from_electric_breath;
extern int gsn_protection_from_acid_breath;
extern int gsn_protection_from_gas_breath;
extern int gsn_anti_magic_shell;
extern int gsn_paralyze;
extern int gsn_curse;

extern int gsn_climb;
extern int gsn_doorbash;
extern int gsn_dual_wield;
extern int gsn_possess;
extern int gsn_blindness;
extern int gsn_poison;
extern int gsn_backstab;
extern int gsn_berserk;
extern int gsn_fireball;
extern int gsn_chill_touch;
extern int gsn_lightning_bolt;
extern int gsn_sleep;
extern int gsn_group_invis;
extern int gsn_invis;
extern int gsn_sneak;
extern int gsn_hunt;
extern int gsn_hide;
extern int gsn_steal;
extern int gsn_spot;
extern int gsn_bash;
extern int gsn_mount;
extern int gsn_disarm;
extern int gsn_kick;
extern int gsn_brew;
extern int gsn_cook;
extern int gsn_faerie_fire;
extern int gsn_shield;
extern int gsn_swim;
extern int gsn_retreat;
extern int gsn_spy;
extern int gsn_travelling;

extern int gsn_comprehend_lang;
extern int gsn_esp;
extern int gsn_memorize;

/* had / need looking at */
extern int gsn_find_traps;
extern int gsn_detrap;
extern int gsn_pick_lock;
extern int gsn_dodge;
extern int gsn_brew;
extern int gsn_blast;
extern int gsn_archery;
extern int gsn_rescue;
extern int gsn_quivering_palm;

/* we want / might want */
extern int gsn_feed;
extern int gsn_gouge;
extern int gsn_search;
extern int gsn_dig;
extern int gsn_parry;
extern int gsn_stun;
extern int gsn_punch;
extern int gsn_grip;
extern int gsn_scribe;
extern int gsn_poison_weapon;

/* zoso? */
extern int gsn_claw;
extern int gsn_bite;
extern int gsn_sting;
extern int gsn_tail;
extern int gsn_slice;

extern int gsn_protection_from_fire;
extern int gsn_protection_from_cold;
extern int gsn_protection_from_energy;
extern int gsn_protection_from_electricity;
extern int gsn_cure_blindness;
extern int gsn_cure_poison;
extern int gsn_stone_skin;
extern int gsn_refresh;
extern int gsn_cure_light;
extern int gsn_heal;

extern int gsn_energy_drain;
extern int gsn_earthquake;
extern int gsn_flamestrike;
extern int gsn_harm;
extern int gsn_colour_spray;
extern int gsn_weaken;
extern int gsn_cure_serious;
extern int gsn_cure_critical;
extern int gsn_dispel_magic;
extern int gsn_dispel_evil;

extern int gsn_meditate;
extern int gsn_psiportal;
extern int gsn_scry;
extern int gsn_doorway;
extern int gsn_tan;
extern int gsn_canibalize;
extern int gsn_psishield;
extern int gsn_mindblank;
extern int gsn_tower_of_iron_will;
extern int gsn_great_sight;
extern int gsn_psistrength;
extern int gsn_chameleon;

extern int gsn_slow_poison;
extern int gsn_cause_light;
extern int gsn_cause_serious;
extern int gsn_cause_critical;
extern int gsn_acid_blast;

extern int gsn_juggernaut;
extern int gsn_pray;
extern int gsn_read_magic;
extern int gsn_babel;

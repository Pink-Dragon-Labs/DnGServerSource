//
// skills.hpp
//
// Skill system definitions.
//
// author: Stephen Nichols
//

#ifndef _SKILLS_HPP_
#define _SKILLS_HPP_

enum {
	_SKILL_IGNORANT,
	_SKILL_FAMILIAR,
	_SKILL_PROFICIENT,
	_SKILL_EXPERT,
	_SKILL_MASTER,
	_SKILL_GRAND_MASTER
};

enum {
	_SKILL_LVL_IGNORANT,
	_SKILL_LVL_FAMILIAR,
	_SKILL_LVL_PROFICIENT,
	_SKILL_LVL_EXPERT,
	_SKILL_LVL_MASTER,
	_SKILL_LVL_GRAND_MASTER,
	_SKILL_LVL_MAX
};

enum {
	_SKILL_SHORT_SWORD,					// ability to use short swords
	_SKILL_LONG_SWORD,					// ability to use long swords
	_SKILL_TWOHANDED_SWORD,				// ability to use two-handed swords
	_SKILL_DAGGER,						// ability to use daggers
	_SKILL_AXE,							// ability to use axes
	_SKILL_CLUB,						// ability to use clubs
	_SKILL_MACE,						// ability to use maces
	_SKILL_UNARMED,						// ability to fight with hands
	_SKILL_THROWING,					// ability to throw missile weapons
	_SKILL_ACROBATICS,					// ability to avoid attack
	_SKILL_MAUL,						// ability to use mauls
	_SKILL_SORCERY,						// teleportation magics -- conjuring
	_SKILL_ELEMENTALISM,				// control over elements
	_SKILL_MYSTICISM,					// control over minds
	_SKILL_THAUMATURGY,					// healing / protection magic
	_SKILL_NECROMANCY,					// control over dead / death magic
	_SKILL_THEURGISM,					// knowledge of artifacts
	_SKILL_ALCHEMY,						// potion creation
	_SKILL_WEAPONSMITH,					// create metal weapons / repair weapons
	_SKILL_ARMORER,						// create metal armor / repair armor
	_SKILL_LEATHERWORKER,				// create / repair leather armor
	_SKILL_SEAMSTER,					// create / repair clothing
	_SKILL_TRACKING,					// track / find players or monsters
	_SKILL_HEALER,						// can bind wounds / simple healing
	_SKILL_BROADSWORD,					// ability to use broadswords
	_SKILL_PICK_POCKETS,				// pick pockets
	_SKILL_DETECT_TRAPS,				// detect / disarm traps
	_SKILL_PICK_LOCKS,					// pick locks
	_SKILL_MEDITATION,					// ability to concentrate under pressure
	_SKILL_CRITICAL_STRIKING,			// ability to strike sensitive areas in combat
	_SKILL_SHIELD_USE,					// ability to protect yourself with a shield 
	_SKILL_MAX
};

#endif

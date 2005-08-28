/* $Id: Constants.java,v 1.1 2005-08-28 15:05:08 peter Exp $ */

/**
 * Kiss of Death
 * Constants.java
 * 
 * Game constants.
 */

package src.utils;

public class Constants
{
	// Controller.
    public static final int MIN_PLAYERS = 2;
	public static final int MAX_PLAYERS = 8;
	public static final int MAX_CHARACTERS = 8;

	// Directions.
	public static final int UP_LEFT = 1;
	public static final int UP = 2;
	public static final int UP_RIGHT = 3;
	public static final int RIGHT = 4;
	public static final int DOWN_RIGHT = 5;
	public static final int DOWN = 6;
	public static final int DOWN_LEFT = 7;
	public static final int LEFT = 8;

	// Room constants.
	public static final int ROOM_WIDTH = 26;
	public static final int ROOM_HEIGHT = 16;
	public static final int MAX_ROOMS = 50;

	// Spots contants.
	public static final char WALL = 'w';
	public static final char FLOOR = 'f';
	public static final char DOOR = 'd';
	public static final char HOLE = 'h';
	public static final int SPOT_SIZE = 32;

	// Character strings.
	public static final String ADVENTURER_NAME = "Adventurer";
	public static final String DWARF_NAME = "Dwarf";
	public static final String ELF_NAME = "Elf";
	public static final String WARRIOR_NAME = "Warrior";

	// Characters abbreviations.
	public static final char ADVENTURER = 'a';
	public static final char DWARF = 'd';
	public static final char ELF = 'e';
	public static final char WARRIOR = 'w';
	
	// Attribute abbreviations.
	public static final char ATTR_ARMOR = 'a';
	public static final char ATTR_BROOMSTICK = 'b';
	public static final char ATTR_INVISIBILITY = 'i';
	public static final char ATTR_LIPSTICK = 'l';
	public static final char ATTR_NIGHTVISION = 'n';
	public static final char ATTR_SHIELD = 's';
	public static final char ATTR_SWORD = 'z';
	public static final char ATTR_VITAMINES = 'v';
	public static final char ATTR_ERROR = 0;

	// Gender strings.
	public static final String MALE_NAME = "Male";
	public static final String FEMALE_NAME = "Female";

	// Gender abbreviations.
	public static final char MALE = 'm';
	public static final char FEMALE = 'f';

	// Attributes.
	public static final int MAX_ATTRIBUTES = 4;
	public static final int ATTRIBUTE_CLASSES = 8;
	public static final int ATTRIBUTE_MIN_ROOMS = 2;
	public static final int MAX_ATTRIBUTES_CARRIED = 6;
	public static final int MAX_VITAMINES = 6;

	// Attributes properties.

	// Armor
	public static final int ARMOR_HEALTH = 0;
	public static final int ARMOR_ENERGY = 0;
	public static final int ARMOR_STRENGTH = 30;
	public static final int ARMOR_INTELLIGENCE = 0;
	public static final int ARMOR_SPEED = -5;
	public static final int ARMOR_ARMOR = 50;
	public static final int ARMOR_DURATION = 30;

	// Broomstick
	public static final int BROOMSTICK_HEALTH = 0;
	public static final int BROOMSTICK_ENERGY = 0;
	public static final int BROOMSTICK_STRENGTH = 0;
	public static final int BROOMSTICK_INTELLIGENCE = 0;
	public static final int BROOMSTICK_SPEED = 50;
	public static final int BROOMSTICK_ARMOR = 0;
	public static final int BROOMSTICK_DURATION = 15;

	// Invisibility
	public static final int INVISIBILITY_HEALTH = 0;
	public static final int INVISIBILITY_ENERGY = 0;
	public static final int INVISIBILITY_STRENGTH = 0;
	public static final int INVISIBILITY_INTELLIGENCE = 0;
	public static final int INVISIBILITY_SPEED = 0;
	public static final int INVISIBILITY_ARMOR = 0;
	public static final int INVISIBILITY_DURATION = 20;

	// Lipstick
	public static final int LIPSTICK_HEALTH = 0;
	public static final int LIPSTICK_ENERGY = 0;
	public static final int LIPSTICK_STRENGTH = 0;
	public static final int LIPSTICK_INTELLIGENCE = 0;
	public static final int LIPSTICK_SPEED = 0;
	public static final int LIPSTICK_ARMOR = 0;
	public static final int LIPSTICK_DURATION = 20;

	// Nightvision
	public static final int NIGHTVISION_HEALTH = 0;
	public static final int NIGHTVISION_ENERGY = 0;
	public static final int NIGHTVISION_STRENGTH = 0;
	public static final int NIGHTVISION_INTELLIGENCE = 0;
	public static final int NIGHTVISION_SPEED = 0;
	public static final int NIGHTVISION_ARMOR = 0;
	public static final int NIGHTVISION_DURATION = 10;

	// Shield
	public static final int SHIELD_HEALTH = 0;
	public static final int SHIELD_ENERGY = 0;
	public static final int SHIELD_STRENGTH = 50;
	public static final int SHIELD_INTELLIGENCE = 0;
	public static final int SHIELD_SPEED = -10;
	public static final int SHIELD_ARMOR = 0;
	public static final int SHIELD_DURATION = 30;

	// Sword
	public static final int SWORD_HEALTH = 0;
	public static final int SWORD_ENERGY = 0;
	public static final int SWORD_STRENGTH = 15;
	public static final int SWORD_INTELLIGENCE = 0;
	public static final int SWORD_SPEED = 0;
	public static final int SWORD_ARMOR = 0;
	public static final int SWORD_DURATION = 20;

	// Vitamines
	public static final int VITAMINES_HEALTH = 50;
	public static final int VITAMINES_ENERGY = 50;
	public static final int VITAMINES_STRENGTH = 10;
	public static final int VITAMINES_INTELLIGENCE = 0;
	public static final int VITAMINES_SPEED = 0;
	public static final int VITAMINES_ARMOR = 10;
	public static final int VITAMINES_DURATION = 0;	

	// Movement.
	public static final int ENERGY_MOVEMENT_FLOOR_DECR = 3;
	public static final int ENERGY_MOVEMENT_HOLE_DECR = 6;
	public static final int ENERGY_MOVEMENT_DOOR_DECR = 6;
	public static final int ENERGY_MOVEMENT_INCR = 5;

	// Action constants.
	public static final int DEFENDING_TIME = 2000;
	public static final int MIN_ATTACK_ENERGY = 20;

	// Directories.
	public static final String DIRECTORY_MAPS_BASE = "maps/";
	public static final String DIRECTORY_IMAGES_BASE = "images/";
	public static final String DIRECTORY_SOUNDS_BASE = "sounds/";

	// Game.
    public static final int GAME_DELAY = 30;
    public static final String GAME_NAME = "kod";

    // Message constants.
    public static final int ACTION_YELL = 1;
    public static final int ACTION_CHAT = 2;
    public static final int ACTION_TEAMCHAT = 3;
}

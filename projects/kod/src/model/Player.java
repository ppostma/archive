/* $Id: Player.java,v 1.1 2005-08-28 15:05:07 peter Exp $ */

/**
 * Kiss of Death
 * Player.java
 * 
 * Player definition.
 */

package src.model;

import src.utils.Constants;

public class Player 
{
    private String name;
    private Character character[];

    /**
     * Constructor.
     * Creates a new instance of Character, depending on PlayerType. 
     * 
     * @param name
     * @param playerType
     * @param gender
     */
    public Player(String name, char playerType, char gender) 
    {
		char pt = java.lang.Character.toLowerCase(playerType);
		this.character = new Character[2];
    	this.name = name;

    	switch (pt) {
   		case Constants.ADVENTURER:
   			character[0] = new Adventurer(gender); 
   			break;
		case Constants.DWARF:
			character[0] = new Dwarf(gender); 
			break;
		case Constants.ELF:
			character[0] = new Elf(gender); 
			break;
		case Constants.WARRIOR:
			character[0] = new Warrior(gender); 
			break;
    	}
    }

    /**
	 * Create a monster.
	 */
	public void createMonster()
	{
		Character c = character[0];
		character[0] = new Monster((TeamCharacter)c);
	}

	/**
     * Destroy the current Player. Sets all objects to null.
     */
    public void destroy() 
    {
        character[0].destroy();
    	character[0] = null;

    	if (character[1] != null) {
    	    character[1].destroy();
    	    character[1] = null;
    	}
    	name = null;
    }

    /**
     * Clone the Player. Creates a new instance of Character with an index of 1.
     * 
     * @param cs
     * @return Character  
     */
    public Character clone(CharacterSpot cs) 
    {
    	Character ch = character[0];
		Character clone = null;
    	char Gender = ch.getGender();
    	
    	/* The monster, dead characters and frogs can't clone. */
    	if (ch instanceof Monster || ch.isDead() || ((TeamCharacter)ch).isFrog())
    		return null;

    	if (ch instanceof Adventurer)
    		clone = new Adventurer(Gender);
    	else if (ch instanceof Dwarf)
			clone = new Dwarf(Gender);
		else if (ch instanceof Elf)
			clone = new Elf(Gender);
		else if (ch instanceof Warrior)
			clone = new Warrior(Gender);
		else
		    throw new RuntimeException("Invalid character.");

    	character[1] = clone;
		clone.moveTo(cs, false, 1);

		character[0].cloned(clone);

    	return clone;
    }

    /**
     * Returns the name of the player.
     * 
     * @return String
     */
    public String getName()
    {
        return this.name;
    }

    /**
     * Get the Character with index c.
     * 
     * @param c
     * @return Character
     */
    public Character getCharacter(int c) 
    {
    	return character[c];
    }

    /**
     * Get the Character index while searching for Character.
     * Returns -1 if the Character cannot be found.
     * 
     * @param c
     * @return int
     */
    public int getCharacterNrByCharacter(Character c)
    {
    	for (int i = 0; i < character.length; i++)
    		if (character[i] != null && character[i] == c)
    			return i;

    	return -1;
    }
}

/* $Id: Warrior.java,v 1.1 2005-08-28 15:05:07 peter Exp $ */

/**
 * Kiss of Death
 * Warrior.java
 * 
 * Warrior Character type.
 */

package src.model;

import src.utils.Constants;

public class Warrior extends TeamCharacter 
{
	/**
	 * Constructor.
	 * 
	 * @param gender
	 */
    public Warrior(char gender) 
    {
    	super(gender);
    	this.intelligence -= 30;
    	this.speed -= 40;
    	this.armor -= 10;
    }

    /**
     * Get the player type.
     * 
     * @return Character
     */
    public Character getPlayerType() 
    {
        return this;
    }
    
    /**
     * Get the player type as a char.
     * 
     * @return char
     */
    public char getPlayerTypeChar()
    {
    	return Constants.WARRIOR;
    }

    /**
     * Get the player type as a String.
     * 
     * @return String
     */
    public String getPlayerTypeName()
    {
        return Constants.WARRIOR_NAME;
    }
}

/* $Id: Adventurer.java,v 1.1 2005-08-28 15:05:07 peter Exp $ */

/**
 * Kiss of Death
 * Adventurer.java
 * 
 * Defines the class Adventurer, this is a playing character in the game.
 */

package src.model;

import src.utils.Constants;

public class Adventurer extends TeamCharacter 
{
	/**
	 * Constructor.
	 * 
	 * @param gender
	 */
    public Adventurer(char gender) 
    {
    	super(gender);
    	this.strength -= 20;
    	this.armor -= 20;
    	this.intelligence -= 10;
    	this.speed -= 30;
    }

    /**
     * Get the player type.
     * 
     * @return AttributeChar
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
    	return Constants.ADVENTURER;
    }

    /**
     * Get the player type as a String.
     * 
     * @return String
     */
    public String getPlayerTypeName()
    {
        return Constants.ADVENTURER_NAME;
    }
}

/* $Id: Elf.java,v 1.1 2005-08-28 15:05:07 peter Exp $ */

/**
 * Kiss of Death
 * Elf.java
 * 
 * Elf Character type.
 */

package src.model;

import src.utils.Constants;

public class Elf extends TeamCharacter 
{
	/**
	 * Constructor.
	 * 
	 * @param gender
	 */
    public Elf(char gender) 
    {
    	super(gender);
    	this.strength -= 30;
    	this.armor -= 30;
    	this.intelligence -= 10;
    	this.speed -= 20;
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
    	return Constants.ELF;
    }

    /**
     * Get the player type as a String.
     * 
     * @return String
     */
    public String getPlayerTypeName()
    {
        return Constants.ELF_NAME;
    }
}

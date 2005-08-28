/* $Id: Dwarf.java,v 1.1 2005-08-28 15:05:07 peter Exp $ */

/**
 * Kiss of Death
 * Dwarf.java
 * 
 * Dwarf Character type.
 */

package src.model;

import src.utils.Constants;

public class Dwarf extends TeamCharacter 
{
	/**
	 * Constructor.
	 * 
	 * @param gender
	 */
    public Dwarf(char gender) 
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
    	return Constants.DWARF;
    }

    /**
     * Get the player type as a String.
     * 
     * @return String
     */
    public String getPlayerTypeName()
    {
        return Constants.DWARF_NAME;
    }
}

/* $Id: CharacterSpot.java,v 1.1 2005-08-28 15:05:07 peter Exp $ */

/**
 * Kiss of Death
 * CharacterSpot.java
 * 
 * Character spot, to be implemented by Door, Floor, Hole.
 */

package src.model;

public abstract class CharacterSpot extends Spot 
{
    private Character character;

	/**
	 * Constructor.
	 */
    public CharacterSpot() 
    {
    	character = null;
    }

	/**
	 * Returns the character. Returns null if there's no character.
	 * 
	 * @return Character
	 */
    public Character getCharacter() 
    {
        return character;
    }
    
 	/**
	 * Set the character if there's no-one on the spot.
	 * 
	 * @param c
	 * @return boolean
	 */
    public boolean setCharacter(Character c) 
    {
    	if (character != null && c != null)
    		return false;

    	character = c;
		return true;
    }
}

/* $Id: Floor.java,v 1.1 2005-08-28 15:05:07 peter Exp $ */

/**
 * Kiss of Death
 * Floor.java
 * 
 * Floor spot.
 */

package src.model;

import src.utils.Constants;

public class Floor extends CharacterSpot 
{
    private Attribute attribute;

    /**
     * Constructor.
     */
    public Floor() 
    {
    	attribute = null;
    }

    /**
     * Pick up an attribute from the floor. The attribute will be set
     * to null and the picked up attribute will be returned.
     * If the floor spot doesn't contain an attribute then it will return null.
     * 
     * @return Attribute
     */
    public Attribute pickUpAttribute() 
    {
    	Attribute a = attribute;
    	attribute = null;
    	return a;
    }

    /**
     * Put an attribute on the floor.
     * 
     * @param attr
     */
    public void putAttribute(Attribute attr) 
    {
    	attribute = attr;
    }

    /**
     * Returns the attribute.
     * 
     * @return Attribute
     */
    public Attribute getAttribute()
    {
    	return attribute;
    }

	/**
	 * Returns the defined char for floor.
	 * 
	 * @return char
	 */
	public char getSpotChar()
	{
		return Constants.FLOOR;
	}
}

/* $Id: Door.java,v 1.1 2005-08-28 15:05:07 peter Exp $ */

/**
 * Kiss of Death
 * Door.java
 * 
 * Door spot.
 */

package src.model;

import src.utils.Constants;

public class Door extends CharacterSpot 
{
	private char link;
	private Room room;
	private Door door;

	/**
	 * Constructor.
	 * 
	 * @param room
	 * @param link
	 */
	public Door(Room room, char link)
	{
		this.room = room;
		this.link = link;
	}

	/**
	 * Returns the door link as char.
	 *  
	 * @return char
	 */
	public char getDoorChar()
	{
		return this.link;
	}

	/**
	 * Sets a door.
	 * 
	 * @param door
	 */
    public void setDoor(Door door) 
    {
    	this.door = door;
    }

    /**
     * Returns the room where the door resides.
     * 
     * @return Room
     */
    public Room getRoom()
    {
    	return this.room;
    }

    /**
     * Returns the corresponding door.
     * 
     * @return Door
     */
    public Door getDoor()
    {
    	return this.door;
    }

    /**
     * Returns the defined char for door.  
     * 
     * @return char
     */
	public char getSpotChar()
	{
		return Constants.DOOR;
	}    	
}

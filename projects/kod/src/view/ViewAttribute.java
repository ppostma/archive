/* $Id: ViewAttribute.java,v 1.1 2005-08-28 15:05:08 peter Exp $ */

/**
 * Kiss of Death
 * ViewAttribute.java
 *
 * Attributes class for the view.
 * (this is NOT equal to the Attributes class in the model!!)
 */

package src.view;

import java.awt.Point;

public class ViewAttribute
{
	public char type;
	public int roomID;
	public Point pos;
	
	/**
	 * Constructor. Requires the room ID, the type and the position in the room.
	 *
	 * @param roomID
	 * @param type
	 * @param pos
	 */
	public ViewAttribute(int roomID, char type, Point pos)
	{
		this.roomID = roomID;
		this.type = type;
		this.pos = pos;
	}
}

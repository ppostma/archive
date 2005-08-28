/* $Id: Room.java,v 1.1 2005-08-28 15:05:07 peter Exp $ */

/**
 * Kiss of Death
 * Room.java
 * 
 * Definition for a room in a playfield.
 */

package src.model;

import java.awt.Point;
import java.util.ArrayList;

import src.persistence.RoomReader;
import src.utils.Constants;

public class Room 
{
	private int width, height;
    private Spot spot[][];

    /**
	 * Constructor.
	 * 
	 * @param name
	 */
    public Room(String name) 
    {
    	width = Constants.ROOM_WIDTH;
		height = Constants.ROOM_HEIGHT;

    	RoomReader rr = new RoomReader(name);
    	if (rr.Read() == false) {
    		System.err.println(rr.getError());
    		System.exit(1);
    	}

    	char [][] room = rr.getRoom();

		spot = new Spot[width][height];

		for (int y = 0; y < height; y++)
			for (int x = 0; x < width; x++) {
			    char ch = room[x][y];

			    switch (ch) {
				case Constants.WALL:
					spot[x][y] = new Wall();
					break;
				case Constants.FLOOR:
					spot[x][y] = new Floor();
					break;
				case Constants.HOLE:
				    spot[x][y] = new Hole();
					break;
				default: // It's a door/teleporter
					spot[x][y] = new Door(this, ch);
					break;
				}
			}
    }

    /**
     * Return the room width.
     * 
     * @return int
     */
    public int getWidth()
    {
    	return width;
    }

    /**
     * Return the room height.
     * 
     * @return int
     */
    public int getHeight()
    {
    	return height;
    }

	/**
	 * Get the surrounding CharacterSpot.
	 * Returns null if there's no spot or a non-CharacterSpot (e.g. Wall).
	 *
	 * @param fromSpot
	 * @param direction
	 * @return CharacterSpot
	 */
    public CharacterSpot getSurroundingCharacterSpot(CharacterSpot fromSpot, int direction) 
    {
    	int y = 0, x = 0;
    	Spot sp = null;

    	/* Find the X and Y position of FromSpot. */ 
    	for (int yp = 0; yp < height; yp++) {
			for (int xp = 0; xp < width; xp++) {
				if (fromSpot == spot[xp][yp]) {
					y = yp;
					x = xp;
					sp = spot[xp][yp];
					break;
				}
			}
		}

    	/* Spot not found? */
		if (sp == null)
        	return null;

		/* Move the player according to the Direction. */
        switch (direction) {
        case Constants.UP_LEFT:
			if (--x < 0 || --y < 0)
				return null;
			break;
		case Constants.UP:
			if (--y < 0)
				return null;
			break;
		case Constants.UP_RIGHT:
			if (--y < 0 || ++x >= width)
				return null;       
			break;
		case Constants.RIGHT:
       		if (++x >= width)
       			return null;
			break;
		case Constants.DOWN_RIGHT:
       		if (++y >= height || ++x >= width)
       			return null;
			break;
		case Constants.DOWN:
			if (++y >= height)
				return null;       
			break;
		case Constants.DOWN_LEFT:
       		if (++y >= height || --x < 0)
       			return null;
			break;
		case Constants.LEFT:
			if (--x < 0)
				return null;
			break;
        default:
        	System.err.println("Invalid direction (" + direction + ").");
        }

        /* We cannot move onto a non-CharacterSpot. */
        if (!(spot[x][y] instanceof CharacterSpot))
            return null;

        return (CharacterSpot)spot[x][y];
    }

	/**
	 * Get all characters in this room.
	 * 
	 * @return Character[]
	 */
    public Character[] getCharacters() 
    {
    	ArrayList list = new ArrayList();
    	Character[] ac;
		int x, y;

		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				if (spot[x][y] instanceof CharacterSpot) {
					Character cp = ((CharacterSpot)spot[x][y]).getCharacter();

					if (cp != null)
						list.add(cp);
				}
			}
		}

		ac = new Character[list.size()];

		for (int i = 0; i < list.size(); i++)
			ac[i] = (Character)list.get(i);

		return ac;
    }

	/**
	 * Get the measurement of the room.
	 * 
	 * @return Point
	 */
    public Point getMeasurement()
    {
        return new Point(width, height);
    }

    /**
     * Get all spots in the room.
     * 
     * @return Spot[][]
     */
    public Spot[][] getSpots()
    {
    	return spot;
    }

    /**
     * Count how many attributes the room has.
     * 
     * @return int
     */
    public int getAttributeCount()
    {
    	Spot spots[][] = this.getSpots();
    	int total = 0;

		/* Walk through all spots in the room. */
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				if (spots[x][y] instanceof Floor) {
					/* Check if there's an attribute on the floor. */
					if (((Floor)spots[x][y]).getAttribute() != null)
						total++;
				}
			}
		}
		return total;
    }

    /**
     * Count how many vitamines the room has.
     * 
     * @return int
     */
    public int getVitaminesCount()
    {
        Spot spots[][] = this.getSpots();
        int total = 0;
        
		/* Walk through all spots in the room. */
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				if (spots[x][y] instanceof Floor) {
					/* Check if there's a vitamines attribute on the floor. */
				    Attribute a = ((Floor)spots[x][y]).getAttribute(); 
					if (a != null && a instanceof Vitamines)
						total++;
				}
			}
		}
        return total;
    }

    /**
     * Return a random CharacterSpot.
     * 
     * @return CharacterSpot
     */
    public CharacterSpot getRandomCharacterSpot()
    {
    	int x = (int)Math.round(Math.random() * (width - 1));
    	int y = (int)Math.round(Math.random() * (height - 1));
    	Spot s = spot[x][y];

    	if (s instanceof CharacterSpot)
    		return (CharacterSpot)s;

    	return getRandomCharacterSpot();
    }

    /**
     * Return a random Floor in the room.
     * 
     * @return Floor
     */
    public Floor getRandomFloor()
    {
    	CharacterSpot cs;

		do {
			cs = getRandomCharacterSpot();
		} while (!(cs instanceof Floor));

    	return (Floor)cs;
    }

    /**
     * Put an attribute on a random floor.
     * Returns the Floor on where the attribute has been placed.
     * 
     * @param a
     * @return Floor
     */
    public Floor createAttribute(Attribute a)
    {
    	Floor floor;

    	do {
    	    floor = getRandomFloor();
    	} while (floor.getAttribute() != null);

    	floor.putAttribute(a);

    	return floor;
    }

    /**
     * Get the CharacterSpot on which Character ca is on.
     *  
     * @param ca
     * @return CharacterSpot
     */
    public CharacterSpot getCharacterSpotByCharacter(Character ca)
    {
    	for (int y = 0; y < height; y++) {
    		for (int x = 0; x < width; x++) {
    			Spot s = spot[x][y];
    			if (s instanceof CharacterSpot) {
    				CharacterSpot cs = (CharacterSpot)s;

    				if (cs.getCharacter() == ca)
    					return cs;
				}
			}
		}
    	return null;
    }
}

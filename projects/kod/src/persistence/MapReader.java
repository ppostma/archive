/* $Id: MapReader.java,v 1.1 2005-08-28 15:05:07 peter Exp $ */

/**
 * Kiss of Death
 * MapReader.java
 * 
 * Map reading abstraction. 
 */

package src.persistence;

import java.io.File;
import java.util.ArrayList;

public class MapReader
{
	private String mapdir, error;
	private String roomfile[];

	/**
	 * Constructor.
	 * 
	 * @param name
	 */
    public MapReader(String name)
    {
		this.mapdir = name;
		this.error = null;
    }

    /**
     * Returns the filename for a room.
     * 
     * @param nr
     * @return String
     */
    private String getRoomFilename(int nr)
    {
    	return new String(mapdir + "/room" + nr + ".txt");
    }

    /**
     * Read the map directory, returns false and sets this.error if it fails.
     *  
     * @return boolean
     */
    public boolean Read()
    {
    	if (!(new File(mapdir).isDirectory())) {
    		this.error = "Specified location is not a directory.";
    		return false;
    	}

    	ArrayList roomfiles = new ArrayList();
    	int currentRoom = 1;

    	while (new File(getRoomFilename(currentRoom)).exists()) {
    		roomfiles.add(getRoomFilename(currentRoom));
    		currentRoom++;
    	}

    	roomfile = new String[roomfiles.size()];
    	for (int i = 0; i < roomfiles.size(); i++)
    		roomfile[i] = new String((String)roomfiles.get(i));

    	return true;
    }

    /**
     * Get the room names for this map.
     * 
     * @return String[]
     */
    public String[] getRoomNames()
    {
        return roomfile;
    }

    /**
     * Get the amount of rooms.
     * 
     * @return int
     */
    public int getRoomCount()
    {
    	return roomfile.length;
    }

	/**
	 * Returns an error message if an error is set, otherwise an empty string.
	 * 
	 * @return String
	 */
	public String getError()
	{
		if (this.error == null)
			return "";

		return this.error;
	}

	/**
     * Returns all map names.
     * 
     * @param location
     * @return String[]
     */
    public static String[] getMapNames(String location)
    {
        ArrayList list = new ArrayList();
        File f = new File(location);
        String[] files = f.list();

        for (int i = 0; i < files.length; i++) {
            if (files[i].equals("CVS"))
                continue;
            if (!(new File(location + files[i]).isDirectory()))
                continue;
            list.add(files[i]);
        }

        String[] maps = new String[list.size()];
        for (int i = 0; i < list.size(); i++)
            maps[i] = new String((String)list.get(i));

        return maps;
    }
}

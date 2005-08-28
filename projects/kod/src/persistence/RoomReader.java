/* $Id: RoomReader.java,v 1.1 2005-08-28 15:05:07 peter Exp $ */

/**
 * Kiss of Death
 * RoomReader.java
 * 
 * Room reading abstraction. 
 */

package src.persistence;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;

import src.utils.Constants;

public class RoomReader
{
	private String filename, error;

	private char room[][];

	/**
	 * Constructor.
	 * 
	 * @param name
	 */
	public RoomReader(String name)
	{
		this.filename = name;
		this.error = null;
	}

	/**
	 * Read the room, returns false and sets this.error if it fails.
	 * 
	 * @return boolean
	 */
	public boolean Read()
	{
	    FileReader input;
	    BufferedReader br;
		int width, height;
		boolean success;
		int ch;

		try {
			input = new FileReader(filename);
		} catch (FileNotFoundException e) {
			this.error = "File `" + filename + "' not found.";
			return false;
		}
		br = new BufferedReader(input);

		width  = 0;
		height = 0;
		success = true;

		room = new char[Constants.ROOM_WIDTH][Constants.ROOM_HEIGHT];

		try {
			while ((ch = br.read()) != -1) {
			    if (ch == 13)
					continue;
				if (ch == 10) {
					width = 0;
					height++;
					continue;
				}
				if (width >= Constants.ROOM_WIDTH) {
				    this.error = "Room width is too big, max is " + Constants.ROOM_WIDTH + " characters.";
				    success = false;
				    break;
				}
				if (height >= Constants.ROOM_HEIGHT) {
				    this.error = "Room height is too big, max is " + Constants.ROOM_HEIGHT + " characters.";
				    success = false;
					break;
				}
				room[width++][height] = (char)ch;
			}
		} catch (IOException e) {
			this.error = "I/O Exception.";
			success = false;
		}

		try {
			br.close();
			input.close();
		} catch (IOException e) {
			/* Not a fatal error. */
			System.err.println("Error while closing stream(s): " + e.getMessage());
		}

		return success;
	}

	/**
	 * Get the room array.
	 * 
	 * @return char[][]
	 */
	public char[][] getRoom()
	{
		return room;
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
}

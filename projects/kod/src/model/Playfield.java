/* $Id: Playfield.java,v 1.1 2005-08-28 15:05:07 peter Exp $ */

/**
 * Kiss of Death
 * Playfield.java
 * 
 * Playfield, the facade class.
 */

package src.model;

import java.util.ArrayList;

import src.controller.Game;
import src.model.timers.AttackTimer;
import src.model.timers.DefendTimer;
import src.model.timers.KissTimer;
import src.model.timers.StartGame;
import src.model.timers.UpdateAttributes;
import src.model.timers.UpdateVitamines;
import src.persistence.MapReader;
import src.utils.Constants;
import src.utils.Dice;
import src.utils.Timer;

public class Playfield
{
	private static Playfield instance = null;
	private Game game = null;

	private Timer countdownTimer;
	private boolean gameStarting;
	private boolean gameStarted;
	private Room room[];
    private Player player[] = new Player[Constants.MAX_PLAYERS];

    /**
	 * Constructor.
	 */
    public Playfield() 
    {
    	countdownTimer = null;

    	gameStarting = false;
    	gameStarted = false;
    }

    /**
     * Get a playfield instance (singleton).
     * 
     * @return Instance of Playfield.
     */
    public static synchronized Playfield getInstance()
    {
    	if (instance == null)
    		instance = new Playfield();
    	return instance;
    }

    /**
     * Set the game instance.
     * 
     * @param g
     */
    public void setGame(Game g)
    {
    	this.game = g;
    }

    /**
     * Return the instance of Game.
     * 
     * @return Instance of Game.
     */
    public Game getGame()
    {
    	return this.game; 	
    }

    /**
     * Read the map and the rooms in it and link the doors.
     * 
     * @param location
     */
    public void readMap(String location)
    {
		/* Instance the mapreader. */
		MapReader mr = new MapReader(location);

    	if (mr.Read() == false) {
    		System.err.println(mr.getError());
    		System.exit(1);
    	}

    	/* Get the room names. */
    	String rooms[] = mr.getRoomNames();

        /* Initialize and create rooms. */
		room = new Room[mr.getRoomCount()];

		for (int i = 0; i < rooms.length; i++)
		    room[i] = new Room(rooms[i]);

		boolean error = false;

		/* Link the doors. */
		for (int i = 0; i < room.length; i++) {
			Room r1 = room[i];
			Spot[][] r1spot = r1.getSpots();

			for (int y = 0; y < r1.getHeight(); y++) {
				for (int x = 0; x < r1.getWidth(); x++) {
					if (r1spot[x][y] instanceof Door) {
						Door d1 = (Door)r1spot[x][y];
						int links = 0;

						for (int j = 0; j < room.length; j++) {
							Room r2 = room[j];
							Spot[][] r2spot = r2.getSpots();

							for (int y2 = 0; y2 < r2.getHeight(); y2++) {
								for (int x2 = 0; x2 < r1.getWidth(); x2++) {
									if (r2spot[x2][y2] instanceof Door) {
										Door d2 = (Door)r2spot[x2][y2];
										if (d1 != d2 && d1.getDoorChar() == d2.getDoorChar()) {
											links++;
											d1.setDoor(d2);
											d2.setDoor(d1);
										}
									}
								}
							}
						}

						if (links == 0) {
							error = true;
							System.err.println("Door ("+d1.getDoorChar()+") in room "+(i+1)+" at position ("+x+", "+y+") is unlinked.");
						} else if (links > 1) {
							error = true;
							System.err.println("Door ("+d1.getDoorChar()+") in room "+(i+1)+" at position ("+x+", "+y+") has more than one link.");
						}
					}
				}
			}
		}

		if (error)
    		System.exit(1);
	}

    /**
     * Get a random Attribute.
     * 
     * @return Attribute
     */
    private Attribute getRandomAttribute()
    {
		Attribute a = null;
		int number = Dice.doThrowNumber(Constants.ATTRIBUTE_CLASSES - 1);

		switch (number) {
		case 0:
			a = new Sword();
			break;
		case 1:
			a = new Lipstick();
			break;
		case 2:
			a = new Vitamines();
			break;
		case 3:
			a = new Armor();
			break;
		case 4:
			a = new NightVision();
			break;
		case 5:
			a = new Invisibility();
			break;
		case 6:
			a = new Broomstick();
			break;
		case 7:
			a = new Shield();
			break;
		default:
			throw new RuntimeException("impossible attribute: " + number);
		}

    	return a;
    }

    /**
	 * Places random attributes in random rooms.
	 */
    public synchronized void updateAttributes()
    {
    	int attributes[] = new int[room.length];
    	int total = 0, roomcount = 0;

    	/* Walk through all rooms. */
    	for (int i = 0; i < room.length; i++) {
	        /* Get attribute count of the room. */
	        attributes[roomcount] = room[i].getAttributeCount();
	        total += attributes[roomcount];
	        /* Count the rooms. */
	        roomcount++;
    	}

		/* First check if there are empty rooms. */
    	ArrayList emptyRooms = new ArrayList();

		for (int i = 0; i < roomcount; i++)
			if (attributes[i] == 0)
				emptyRooms.add(new Integer(i));

		/* Add attributes to random rooms, until MAX_ATTRIBUTES. */
    	while (total++ < Constants.MAX_ATTRIBUTES) {
    		int selectedRoom;

    		/* Select an attribute. */ 
    		if (emptyRooms.size() > 0) {
    			/* Try a random empty room first. */
    			int index = (int)(Math.random() * 1000) % emptyRooms.size();
    			selectedRoom = ((Integer)emptyRooms.get(index)).intValue();
    			emptyRooms.remove(index);
    		} else {
    			/* No empty rooms, just select a random room. */
    			selectedRoom = (int)(Math.random() * 1000) % roomcount;
    		}

    		/* Place a random attribute in the selected room. */
    		Floor floor = room[selectedRoom].createAttribute(getRandomAttribute());

    		/* Tell the controller where we placed the new attribute. */
    		getGame().updatePlayfieldAttribute(floor, selectedRoom);
    	}

    	int duration = Dice.doThrowNumber(1000 * 15) + (1000 * 5);
    	new Timer(duration, 1, new UpdateAttributes()).start();
    }

    /**
     * Put a new vitamines attribute in a random room on a random floor.
     */
    public synchronized void updateVitamines()
    {
        int total = 0;

    	/* Walk through all rooms. */
    	for (int i = 0; i < room.length; i++)
    	    total += room[i].getVitaminesCount();

    	/* Don't drop too much vitamines. */
    	if (total >= Constants.MAX_VITAMINES)
    	    return;
    	
        /* Get a random room. */
        int roomID = Dice.doThrowNumber(room.length - 1);

        /* Place vitamines on a random floor. */
        Floor floor = room[roomID].createAttribute(new Vitamines());

        /* Tell the controller where we placed the new attribute. */
        getGame().updatePlayfieldAttribute(floor, roomID);

        int duration = Dice.doThrowNumber(1000 * 20) + (1000 * 10);
    	new Timer(duration, 1, new UpdateVitamines()).start();
    }

    /**
     * Place character on a random spot.
     * Called from signIn and checkAllDead, synchronized.
     * 
     * @param c
     */
    public void placeCharacter(Character c)
    {
		if (c == null)
			return;

		Room r = room[(int)Math.round(Math.random() * (room.length - 1))];

		CharacterSpot cs = null;

		while (cs == null || cs.getCharacter() != null)
			cs = r.getRandomFloor();

		c.moveTo(cs, false, 1);
	}

    /**
     * Check if everyone is dead.
     * This function is called from AttackTimer and is already synchronized.
     */
    public void checkAllDead() 
    {
    	Monster monster = null;
    	int total = 0;

    	for (int i = 0; i < player.length; i++) {
    		if (player[i] != null) {
    			Character character = player[i].getCharacter(0);
    			Character clone = player[i].getCharacter(1);

    			if (character instanceof Monster) {
    				monster = (Monster)character;
    				continue;
    			}
    			if (character != null && !character.isDead())
    				total++;
    			if (clone != null && !clone.isDead())
    				total++;
    		}
    	}

    	/* If the monster dies, let is respawn. */
    	if (monster != null && monster.isDead()) {
    		if (monster.getLocation() != null) { 
    			monster.getLocation().setCharacter(null);
    			monster.setLocation(null);
        	}
    		monster.setAttributes();
        	monster.setAlive();

        	this.placeCharacter(monster);

        	int playerID = getPlayerIDByCharacter(monster);
    		Player p = getPlayerByID(playerID);

    		getGame().characterMoved(playerID, p.getCharacterNrByCharacter(monster));
    		getGame().gameEvent("Mohahaha");
    	}

    	/* If everyone is dead, the monster wins. */
    	if (total == 0) {
    		endGame(monster);
    	}
    }

    /**
     * Checks the player count.
     * This function is called each time when a player logs in or logs off.
     */
    public synchronized void checkPlayerCount()
    {
    	if (!gameStarted) {
        	int count = 0;

        	for (int i = 0; i < player.length; i++)
	    		if (player[i] != null)
	    			count++;

        	if (count >= Constants.MIN_PLAYERS) { 
			    if (!gameStarting) {
			    	gameStarting = true;

			    	countdownTimer = new Timer(1000, Constants.GAME_DELAY, new StartGame());
					countdownTimer.start();

					getGame().countDown(Constants.GAME_DELAY);

					System.out.println("Count down started.");
			    }
			} else {
				if (gameStarting) {
					if (countdownTimer != null) {
						countdownTimer.stop();
						countdownTimer = null;
					}
					gameStarting = false;

					getGame().countDown(-1);	/* Reset the counter. */
				}
			    System.out.println("Waiting for " + (Constants.MIN_PLAYERS - count) + " players.");
			}
    	} else {
    		/*
    		 * Someone leaves while the game is active. If the monster leaves then
    		 * end the game immediately. If the player count is lower than 2
    		 * then end the game as well.
    		 */
    		boolean monster = false;
    		int count = 0;

        	for (int i = 0; i < player.length; i++) {
	    		if (player[i] != null) {
	    			if (player[i].getCharacter(0) instanceof Monster)
	    				monster = true;
	    			count++;
	    		}
        	}
        	if (monster == false || count < 2)
        		endGame(null);
    	}
    }

    /**
     * Sign in to the game. Returns the unique number to identify the player.
     * Returns -1 if the game is full.
     * 
     * @param name
     * @param playerType
     * @param gender
     * @return int
     */
    public synchronized int signIn(String name, char playerType, char gender) 
    {
		int playerID = -1;

		/* Can't join active games. */
		if (gameStarted) {
    		System.out.println("Player " + name + " can't sign in, the game is already started.");
			return -1;
		}

		/* Find an unused player slot. */
		for (int i = 0; i < Constants.MAX_PLAYERS; i++) {
			if (player[i] == null) {
				playerID = i;
				break;
			}
		}

		/* Can be -1 if the game is full. */
    	if (playerID == -1) {
    		System.out.println("Player " + name + " can't sign in, the game is full.");
			return -1;
    	}

    	player[playerID] = new Player(name, playerType, gender);

		placeCharacter(player[playerID].getCharacter(0));

		System.out.println("Player " + name + " signed in.");

		return playerID;
    }

    /**
     * Sign out of the game.
     * 
     * @param playerID
     */
    public synchronized void signOut(int playerID) 
    {
        Player p = player[playerID];

        System.out.println("Player " + p.getName() + " signed out.");

        if (p != null) {
            p.destroy();
            player[playerID] = null;
        }
    }

	/**
	 * Creates the monster.
	 * 
	 * @return playerID of the monster
	 */
	public synchronized int assignMonster()
	{
		ArrayList al = new ArrayList();
		int players = 0;

		for (int i = 0; i < player.length; i++)
			if (player[i] != null)
				al.add(new Integer(i));

		int monster = ((Integer)al.get((int)(Math.random() * players))).intValue();

		Player p = getPlayerByID(monster);
		p.createMonster();

		return monster;
	}

	/**
	 * Start the game.
	 */
	public synchronized void startGame()
    {
    	gameStarted = true;
    	getGame().startGame();

    	System.out.println("Game started.");
    }

    /**
     * End the game.
     * 
     * @param ch
     */
    public synchronized void endGame(Character ch)
    {
        gameStarted = false;
        getGame().endGame(ch);

        if (ch != null) {
        	int playerID = getPlayerIDByCharacter(ch);
        	System.out.println("Game ended, winner is " + player[playerID].getName() + ".");
    	} else {
    		System.out.println("Game ended, no winner.");
    	}
    }

    /**
     * Clone the Player.
     * 
     * @param playerID
     * @param direction
     * @return Character
     */
    public synchronized Character clone(int playerID, int direction) 
    {
    	/* Allow cloning only once. */
    	if (player[playerID].getCharacter(1) != null)
    		return null;

    	Player p = player[playerID];
    	if (p == null)
    		return null;
    	
    	Character c = p.getCharacter(0);
    	if (c == null)
    		return null;

    	Room r = getRoomByCharacter(c);
    	if (r == null)
    		return null;

    	CharacterSpot fromSpot = c.getLocation(); 

    	/* Clone the character if the spot is available. */
		if (fromSpot != null) {
			CharacterSpot cs = r.getSurroundingCharacterSpot(fromSpot, direction);
			if (cs != null)
				return player[playerID].clone(cs);
		}

		return null;
    }

    /**
     * Attack a spot. Returns true if the character is going to attack.
     * 
     * @param fromPlayer
     * @param fromCharacter
     * @param direction
     * @return boolean
     */
    public synchronized boolean attack(int fromPlayer, int fromCharacter, int direction) 
    {
    	Character attacker = player[fromPlayer].getCharacter(fromCharacter);
    	if (attacker == null)
    	    return false;

    	if (attacker.getEnergy() < Constants.MIN_ATTACK_ENERGY)
    		return false;
    	
    	/* Do only one action at once. */
    	if (attacker.isAction())
    		return false;
    	
	    attacker.setAttacking(true);

	    Room r = getRoomByCharacter(attacker);
	    if (r == null)
	    	return false;
	    
	    CharacterSpot cs = r.getSurroundingCharacterSpot(attacker.getLocation(), direction);

	    int duration = src.utils.Dice.doThrowNumber(1000);
	    new Timer(duration, 1, new AttackTimer(attacker, cs)).start();

	    return true;
    }

    /**
     * Defend. Return true if the character is going to defend.
     * 
     * @param fromPlayer
     * @param fromCharacter
     * @param direction
     */
    public synchronized void defend(int fromPlayer, int fromCharacter, int direction) 
    {
    	Character defender = player[fromPlayer].getCharacter(fromCharacter);
    	if (defender == null)
    	    return;

    	/* Do only one action at once. */
		if (defender.isAction())
			return;

    	defender.setDefending(true);
	    
		Room r = getRoomByCharacter(defender);
		if (r == null)
		    return;

		CharacterSpot cs = r.getSurroundingCharacterSpot(defender.getLocation(), direction);
		if (cs == null)
		    return;

	    int duration = Dice.doThrowNumber(500);
	    new Timer(duration, 1, new DefendTimer(cs, defender)).start();
    }

    /**
     * Move to another spot. Returns true if the move was succesful or false if
     * we cannot move to the spot (e.g. a non-Characterspot or there's already
     * someone else on it).
     * 
     * @param playerID
     * @param characterNr
     * @param direction
     * @return boolean
     */
    public synchronized boolean moveTo(int playerID, int characterNr, int direction) 
    {
		Player p = player[playerID];
		if (p == null)
		    return false;

		Character c = p.getCharacter(characterNr);
		if (c == null)
		    return false;

		if (!c.isDead() && c != null) {
			Room r = getRoomByCharacter(c);
			if (r == null)
			    return false;
			
			boolean moved = c.moveTo(r.getSurroundingCharacterSpot(c.getLocation(), direction), true, 1); 
			if (moved)
				c.startMoveTimer();
		
			return moved;
		}
		return false;
    }

    /**
     * Run to another spot. Returns true if the move was succesful or false if
     * we cannot move to the spot (e.g. a non-Characterspot or there's already
     * someone else on it).
     * 
     * @param playerID
     * @param characterNr
     * @param direction
     * @return boolean
     */
    public synchronized boolean runTo(int playerID, int characterNr, int direction) 
    {
		boolean moved = false;
		Player p = player[playerID];
		if (p == null)
		    return false;

		Character c = p.getCharacter(characterNr);
		if (c == null)
		    return false;

		Room r = getRoomByCharacter(c);
		if (r == null)
		    return false;

		/* Do 2 move to's. */
		for (int i = 0; i < 2; i++) {
			CharacterSpot s = r.getSurroundingCharacterSpot(c.getLocation(), direction);
		    if (c.moveTo(s, true, 2))
		    	moved = true;
		}

		if (moved)
			c.startMoveTimer();

		return moved;
    }

    /**
     * Kiss another Character.
     * Returns true if the character is kissing. 
     * 
     * @param fromPlayer
     * @param fromCharacter
     * @param direction
     * @return boolean
     */
    public synchronized boolean kiss(int fromPlayer, int fromCharacter, int direction) 
    {
		Character kisser = player[fromPlayer].getCharacter(fromCharacter);
		if (kisser == null)
			return false;

		Room r = getRoomByCharacter(kisser);
		if (r == null)
			return false;

		CharacterSpot cs = r.getSurroundingCharacterSpot(kisser.getLocation(), direction);

		/* Check if there is an Character at that spot, if so, kiss him/her. */
		if (cs != null) {
			Character kissed = cs.getCharacter();
			if (kissed != null && kisser instanceof TeamCharacter) {
			    /* Do only one action at once. */
			    if (!kissed.isAction()) {
				    /* Enable kissing mode. */
			        kisser.setKissing(true);

			        /* Setup a timer for the kiss. */ 
				    int duration = Dice.doThrowNumber(1500) + 500;
				    new Timer(duration, 1, new KissTimer(kisser, kissed)).start();

				    return true;
			    }
			}
		}

		return false;
    }

    /**
     * Pick up an attribute. Returns the attribute ID that we've picked up
     * or -1 if we didn't picked up anything. 
     * 
     * @param playerID
     * @param characterNr
     * @return int
     */
    public synchronized int pickUp(int playerID, int characterNr) 
    {
    	Character c = player[playerID].getCharacter(characterNr);
    	if (c == null)
    		return -1;

    	return c.pickUp();
    }

    /**
     * Use an attribute. Returns the used attribute.
     * 
     * @param playerID
     * @param characterNr
     * @param attributeNr
     * @return boolean
     */
    public synchronized Attribute useAttribute(int playerID, int characterNr, int attributeNr) 
    {
		Character c = player[playerID].getCharacter(characterNr);
		if (c == null)
			return null;
		
		return c.useAttribute(attributeNr);
    }

    /**
     * Drop an attribute.
     * 
     * @param playerID
     * @param characterNr
     * @param attributeNr
     * @return Attribute
     */
    public synchronized Attribute dropDown(int playerID, int characterNr, int attributeNr) 
    {
		Character c = player[playerID].getCharacter(characterNr);
		if (c == null)
			return null;

		return c.dropDown(attributeNr);
    }

    /**
     * Team chat.
     * 
     * @param playerID
     * @param message
     */
    public synchronized void teamChat(int playerID, String message) {
    	Player p = getPlayerByID(playerID);
		Character teamType = p.getCharacter(0).getPlayerType();
		String playerName = p.getName();

		for (int i = 0; i < Constants.MAX_PLAYERS; i++) {
			Player p2 = getPlayerByID(i);
			if (p2 != null) {
				Character characterType = p2.getCharacter(0).getPlayerType();
				if (characterType.getPlayerTypeChar() == teamType.getPlayerTypeChar())
					if (player[i] != null)
						getGame().sendToWatcher(Constants.ACTION_TEAMCHAT, i, playerName, message);
			}
		}
    }

    /**
     * Yell (talk in only one room).
     * 
     * @param playerID
     * @param characterNr
     * @param message
     */
    public synchronized void yellChat(int playerID, int characterNr, String message)
    {
    	Player p1 = getPlayerByID(playerID);
    	String playerName = p1.getName();
        Character ch = getPlayerByID(playerID).getCharacter(characterNr);
        if (ch == null)
        	return;

        int roomID = getRoomIDByCharacter(ch);
        if (roomID == -1)
        	return;

        Room rooms[] = getRooms();
		Character[] ac = rooms[roomID].getCharacters();

		for (int i = 0; i < ac.length; i++) {
			for (int j = 0; j < Constants.MAX_PLAYERS - 1; j++){
				Player p2 = getPlayerByID(j);
				if (p2 != null) {
					Character char1 = p2.getCharacter(0);
					Character char2 = p2.getCharacter(1);

					if ((ac[i] == char1 || ac[i] == char2) && message != null)
						getGame().sendToWatcher(Constants.ACTION_YELL, j, playerName, message);
				}
			}
		}
    }

    /**
     * Chat to everyone in the game.
     * 
     * @param playerID
     * @param message
     */
    public synchronized void allChat(int playerID, String message)
    {
    	Player p = getPlayerByID(playerID);
		String playerName = p.getName();

		for (int i = 0; i < Constants.MAX_PLAYERS; i++)
			if (player[i] != null)
				getGame().sendToWatcher(Constants.ACTION_CHAT, i, playerName, message);
    }

    /**
     * Returns the instance of Player with ID (index).
     * 
     * @param index
     * @return Player
     */
    public synchronized Player getPlayerByID(int index)
    {
    	if (player[index] != null)
    		return player[index];

    	return null;
    }

    /**
     * Get the Player ID (index) while searching for the Character.
     * Returns -1 if the Character cannot be found. 
     * 
     * @param c
     * @return int
     */
    public synchronized int getPlayerIDByCharacter(Character c)
    {
    	for (int i = 0; i < player.length; i++)
    		if (player[i] != null && player[i].getCharacterNrByCharacter(c) != -1)
    			return i;

    	return -1;
    }

    /**
     * Get a room by ID (index).
     * 
     * @param index
     * @return Room
     */
    public synchronized Room getRoomByID(int index)
    {
    	return room[index];
    }

    /**
     * Get the room in which the Character c is in.
     * Returns null if the Character cannot be found.
     * 
     * @param c
     * @return Room
     */
    public synchronized Room getRoomByCharacter(Character c)
    {
    	if (!c.isDead() && c != null)
    		return getRoomByID(getRoomIDByCharacter(c));

    	return null;
    }

    /**
     * Get the room ID in which the Character c is in.
     * Returns -1 if the Character cannot be found.
     * 
     * @param c
     * @return int
     */
    public synchronized int getRoomIDByCharacter(Character c)
    {
    	/* Walk through the rooms to look where the character is in. */
    	if (c != null && !c.isDead()) {
			for (int i = 0; i < room.length; i++) {
				if (room[i] != null) {
					Character compare[] = room[i].getCharacters();
					for (int j = 0; j < compare.length; j++)
						if (compare[j] == c)
							return i;
				}
			}
    	}
    	return -1;
    }

    /**
     * Returns all rooms.
     * 
     * @return Room[]
     */
    public synchronized Room[] getRooms()
    {
    	return room;
    }

    /**
     * Return all players that have won the game.
     * 
     * @param ch
     * @return int[]
     */
    public synchronized int[] getWinPlayers(Character ch)
    {
    	ArrayList al = new ArrayList();

    	for (int i = 0; i < player.length; i++) {
    		if (player[i] != null) {
    			Character ch2 = player[i].getCharacter(0);
    			if (ch.getPlayerTypeChar() == ch2.getPlayerTypeChar()) {
    				al.add(new Integer(i));
    			}
    		}
    	}

    	int[] win = new int[al.size()];
    	for (int i = 0; i < al.size(); i++)
    		win[i] = ((Integer)al.get(i)).intValue();

    	return win;
    }

    /**
     * Return all players that have lost the game.
     * 
     * @param ch
     * @return int[]
     */
    public synchronized int[] getLostPlayers(Character ch)
    {
    	ArrayList al = new ArrayList();

    	for (int i = 0; i < player.length; i++) {
    		if (player[i] != null) {
    			Character ch2 = player[i].getCharacter(0);
    			if (ch.getPlayerTypeChar() != ch2.getPlayerTypeChar()) {
    				al.add(new Integer(i));
    			}
    		}
    	}

    	int[] win = new int[al.size()];
    	for (int i = 0; i < al.size(); i++)
    		win[i] = ((Integer)al.get(i)).intValue();

    	return win;
    }
}

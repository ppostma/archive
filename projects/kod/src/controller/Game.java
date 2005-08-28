/* $Id: Game.java,v 1.1 2005-08-28 15:05:08 peter Exp $ */

/**
 * Kiss of Death
 * Game.java
 * 
 * This is the controller.
 * This class is also equivalent to the "Observable" in java.
 */

package src.controller;

import java.awt.Point;
import java.rmi.RemoteException;
import java.rmi.server.UnicastRemoteObject;

import src.model.Attribute;
import src.model.Character;
import src.model.CharacterSpot;
import src.model.Floor;
import src.model.Player;
import src.model.Playfield;
import src.model.Room;
import src.model.Spot;
import src.utils.Constants;
import src.watcher.Watchable;
import src.watcher.Watcher;
import src.watcher.WatcherList;

public class Game extends UnicastRemoteObject implements Watchable
{
	private Playfield pf;
	private WatcherList watchers = new WatcherList();

	/**
	 * Constructor.
	 * 
	 * @param map
	 * @throws RemoteException
	 */
	public Game(String map) throws RemoteException
	{
		super();

		pf = Playfield.getInstance();

		pf.setGame(this);
		pf.readMap(map);

		/* Place attributes in the playfield. */
		pf.updateAttributes();
		pf.updateVitamines();
	}

	/**
	 * Sign in function.
	 * - Passes functionality on to the Model.
	 * - Notifies the Watchers.
	 * 
	 * @param w
	 * @param playerName
	 * @param playerType
	 * @param gender
	 * @return int New Player ID
	 * @throws RemoteException
	 */
	public int action_SignIn(Watcher w, String playerName, char playerType, char gender) throws RemoteException
	{
		String genderText = null;
		String characterText = null;

		int playerID = pf.signIn(playerName, playerType, gender);
		if (playerID == -1)
		    return -1;

		watchers.add(w, playerID);

		w.event_SignIn(playerID, getRooms());

		for (int i = 0; i < watchers.size(); i++) {
			try {
				watchers.get(i).event_NewPlayer(playerID, playerName, playerType, gender);
			} catch (RemoteException e) {
				System.err.println("RemoteException in action_SignIn: " + e.getMessage());
			}
		}

		Player p = pf.getPlayerByID(playerID);

		Watcher watcher = watchers.getWatcherByPlayerID(playerID);
		if (watcher != null) {
			for (int i = 0; i < Constants.MAX_PLAYERS; i++) {
				Player p1 = pf.getPlayerByID(i);
				if (p1 != null && p1 != p) {
					Character c1 = p1.getCharacter(0); 
					try {
						watcher.event_NewPlayer(i, p1.getName(), c1.getPlayerTypeChar(), c1.getGender());
					} catch (RemoteException e) {
						System.err.println("RemoteException in action_SignIn: " + e.getMessage());
					}
					characterMoved(i, 0);
				}
				try {
				    updateCharacterAttributes(watcher, playerID, 0);
				} catch (RemoteException e) {
					System.err.println("RemoteException in action_SignIn: " + e.getMessage());
				}
			}
		}

		placePlayfieldAttributes(playerID);
		characterMoved(playerID, 0);

		switch (gender) {
		case Constants.FEMALE:
			genderText = "She";
			break;
		case Constants.MALE:
		    genderText = "He";
			break;			
		}
		characterText = p.getCharacter(0).getPlayerTypeName();

		gameEvent(playerName + " has signed in. " + genderText + " joined as " + characterText + ".");

		pf.checkPlayerCount();

		return playerID;
	}

	/**
	 * Sign off function.
	 * - Passes functionality on to the Model.
	 * - Notifies the Watchers.
	 * 
	 * @param playerID
	 * @throws RemoteException
	 */
    public void action_SignOff(int playerID) throws RemoteException
    {
        Watcher w = watchers.getWatcherByPlayerID(playerID);
        if (w == null) {
            /* Client probably disconnected. */
	        return;
	    }

        watchers.remove(w);

	    gameEvent(pf.getPlayerByID(playerID).getName() + " has left the building.");	

	    pf.signOut(playerID);

	    for (int i = 0; i < watchers.size(); i++) {
	    	try {
	    		watchers.get(i).event_RemovePlayer(playerID);
	    	} catch (RemoteException e) {
	    		System.err.println("RemoteException in action_SignOff: " + e.getMessage());
	    	}
	    }

	    pf.checkPlayerCount();
    }

    /**
     * Update attributes of the character.
     * - Notifies the Watchers.
     * 
     * @param w
     * @param playerID
     * @param characterNr
     * @throws RemoteException
     */
    private void updateCharacterAttributes(Watcher w, int playerID, int characterNr) throws RemoteException
    {
        if (w != null) {
            Player p = pf.getPlayerByID(playerID);
            if (p == null)
                return;
            
            Character c = p.getCharacter(characterNr);

            if (c != null)
   	    	    w.event_CharacterAttributesUpdated(playerID, characterNr, c.getHealth(), c.getEnergy(), c.getStrength(), c.getSpeed(), c.getIntelligence(), c.getArmor(), c.getInvisible(), c.getNightvision());
        }
    }

    /**
	 * Notifies the watchers of movement (move/run).
	 * 
	 * @param playerID
	 * @param characterNr
	 */
	public void characterMoved(int playerID, int characterNr)
	{
		Character c = pf.getPlayerByID(playerID).getCharacter(characterNr);
		Point pos = getCharacterPosition(c);

		if (pos == null) {
	        System.err.println("characterMoved: can't find character position (" + playerID + ", " + characterNr + ")");
		    return;
		}

		for (int i = 0; i < watchers.size(); i++) {
			try {
				watchers.get(i).event_CharacterMoved(playerID, characterNr, pf.getRoomIDByCharacter(c), pos.x, pos.y);
			} catch (RemoteException e) {
				System.err.println("RemoteException in characterMoved: " + e.getMessage());
			}
		}

		Watcher watcher = watchers.getWatcherByPlayerID(playerID);
		if (watcher != null) {
			try {
			    updateCharacterAttributes(watcher, playerID, characterNr);
			} catch (RemoteException e) {
				System.err.println("RemoteException in characterMoved: " + e.getMessage());
			}
		}
	}

	/**
	 * Moves the Character in a direction.
	 * - Passes functionality on to the Model.
	 * - Notifies the Watchers (via characterMoved).
	 * 
	 * @param playerID
	 * @param characterNr
	 * @param direction
	 * @throws RemoteException
	 */
	public void action_MoveCharacter(int playerID, int characterNr, int direction) throws RemoteException
	{
		if (pf.moveTo(playerID, characterNr, direction))
			characterMoved(playerID, characterNr);
	}

	/**
	 * Moves the Character in a direction.
	 * - Passes functionality on to the Model.
	 * - Notifies the Watchers (via characterMoved).
	 * 
	 * @param playerID
	 * @param characterNr
	 * @param direction
	 * @throws RemoteException
	 */
	public void action_RunCharacter(int playerID, int characterNr, int direction) throws RemoteException
	{
		if (pf.runTo(playerID, characterNr, direction))
			characterMoved(playerID, characterNr);
	}

	/**
	 * Character kisses in a direction.
	 * - Passes functionality on to the Model.
	 * - Notifies the Watchers about the kiss.
	 * 
	 * @param playerID
	 * @param characterNr
	 * @param direction
	 * @throws RemoteException
	 */
	public void action_KissCharacter(int playerID, int characterNr, int direction) throws RemoteException
	{
		if (pf.kiss(playerID, characterNr, direction)) {
			for (int i = 0; i < watchers.size(); i++) {
				try {
					watchers.get(i).event_CharacterGoingToKiss(playerID, characterNr);
				} catch (RemoteException e) {
					System.err.println("RemoteException in action_KissCharacter: " + e.getMessage());
				}
			}
		}
	}

	/**
	 * Kiss timer is finished.
	 * - Notifies the Watchers.
	 * 
	 * @param kisser
	 * @param kissed
	 * @param kisserFrog
	 * @param kissedFrog
	 */
	public void kissFinished(Character kisser, Character kissed, boolean kisserFrog, boolean kissedFrog)
	{
		/* Event for the kissing character. */
		int playerID = pf.getPlayerIDByCharacter(kisser);

		if (playerID == -1) {
	        /* Client probably disconnected. */
	        System.err.println("kissFinished: unable to find playerID");
	        return;
	    }

		int characterNr = pf.getPlayerByID(playerID).getCharacterNrByCharacter(kisser);

		for (int i = 0; i < watchers.size(); i++) {
            try {
                watchers.get(i).event_CharacterKissed(playerID, characterNr, kisserFrog);
            } catch (RemoteException e) {
            	System.err.println("RemoteException in kissFinished: " + e.getMessage());
            }
		}

		/* Event for the kissed character. */
	    playerID = pf.getPlayerIDByCharacter(kissed);

	    if (playerID == -1) {
	        /* Client probably disconnected. */
	        System.err.println("kissFinished: unable to find playerID");
	        return;
	    }

	    characterNr = pf.getPlayerByID(playerID).getCharacterNrByCharacter(kissed);

	    for (int i = 0; i < watchers.size(); i++) {
            try {
                watchers.get(i).event_CharacterHasBeenKissed(playerID, characterNr, kissedFrog);
            } catch (RemoteException e) {
            	System.err.println("RemoteException in kissFinished: " + e.getMessage());
            }
		}
	}

	/**
	 * Character defends in a direction.
	 * - Passes functionality on to the Model.
	 * 
	 * @param playerID
	 * @param characterNr
	 * @param direction
	 * @throws RemoteException
	 */
	public void action_Defend(int playerID, int characterNr, int direction) throws RemoteException 
	{
		pf.defend(playerID, characterNr, direction);
	}

    /**
     * Start the defending, called from DefendTimer.
     * 
     * @param defender
     */
    public void defendStarting(Character defender)
    {
        int playerID = pf.getPlayerIDByCharacter(defender);

        if (playerID == -1) {
	        /* Client probably disconnected. */
	        System.err.println("defendStarting: unable to find playerID");
	        return;
	    }

        int characterNr = pf.getPlayerByID(playerID).getCharacterNrByCharacter(defender);

        for (int i = 0; i < watchers.size(); i++) {
            try {
                watchers.get(i).event_CharacterGoingToDefend(playerID, characterNr);
            } catch (RemoteException e) {
            	System.err.println("RemoteException in defendStarting: " + e.getMessage());
            }
        }
    }

    /**
     * Defending has ended, called from DefendTimer.
     * 
     * @param defender
     */
    public void defendFinished(Character defender)
    {
        int playerID = pf.getPlayerIDByCharacter(defender);

        if (playerID == -1) {
	        /* Client probably disconnected. */
	        System.err.println("defendFinished: unable to find playerID");
	        return;
	    }

        int characterNr = pf.getPlayerByID(playerID).getCharacterNrByCharacter(defender);

        for (int i = 0; i < watchers.size(); i++) {
            try {
                watchers.get(i).event_CharacterDefend(playerID, characterNr);
            } catch (RemoteException e) {
            	System.err.println("RemoteException in defendFinished: " + e.getMessage());
            }
        }
    }

    /**
	 * Character attacks in a direction.
	 * - Passes functionality on to the Model.
	 * - Notifies the Watchers.
	 * 
	 * @param playerID
	 * @param characterNr
	 * @param direction
     * @throws RemoteException
	 */
	public void action_Attack(int playerID, int characterNr, int direction) throws RemoteException
	{
		if (pf.attack(playerID, characterNr, direction)) {
			for (int i = 0; i < watchers.size(); i++) {
				try {
					watchers.get(i).event_CharacterGoingToAttack(playerID, characterNr);
				} catch (RemoteException e) {
					System.err.println("RemoteException in action_Attack: " + e.getMessage());
				}
			}
		}
	}

	/**
	 * Character attack finished.
	 * - Notifies the Watchers.
	 * 
	 * @param attacker
	 * @param attacked
	 * @param isDead
	 */
	public void attackFinished(Character attacker, Character attacked, boolean isDead)
	{
	    int playerID = pf.getPlayerIDByCharacter(attacker);

	    if (playerID == -1) {
	        /* Client probably disconnected. */
	        System.err.println("attackFinished: unable to find playerID");
	        return;
	    }

	    int characterNr = pf.getPlayerByID(playerID).getCharacterNrByCharacter(attacker);

		if (attacked != null) {
			int attackedPlayerID = pf.getPlayerIDByCharacter(attacked);	
			int attackedCharacterNr = pf.getPlayerByID(attackedPlayerID).getCharacterNrByCharacter(attacked);

			for (int i = 0; i < watchers.size(); i++) {
                try {
                    watchers.get(i).event_CharacterAttack(playerID, characterNr);
                } catch (RemoteException e) {
                	System.err.println("RemoteException in attackFinished: " + e.getMessage());
                }
			}

			Watcher watcher = watchers.getWatcherByPlayerID(playerID);
			if (watcher != null) {
                try {
				    updateCharacterAttributes(watcher, attackedPlayerID, attackedCharacterNr);
                } catch (RemoteException e) {
                	System.err.println("RemoteException in attackFinished: " + e.getMessage());
                }
			}

			if (isDead) {
                Character c0 = pf.getPlayerByID(attackedPlayerID).getCharacter(0);
                Character c1 = pf.getPlayerByID(attackedPlayerID).getCharacter(1);
                if (c1 != null && c0 != null) {
                	if (c0.isDead() && c1.isDead()) {
                		gameEvent(pf.getPlayerByID(attackedPlayerID).getName() + " has been chopped to death by " + pf.getPlayerByID(pf.getPlayerIDByCharacter(attacker)).getName() +"!");
                	}
                } else if (c0 != null && c1 == null) {
                	if (c0.isDead()) {
                		gameEvent(pf.getPlayerByID(attackedPlayerID).getName() + " has been chopped to death by " + pf.getPlayerByID(pf.getPlayerIDByCharacter(attacker)).getName() +"!");
                	}
                }
			    for (int i = 0; i < watchers.size(); i++) {
                    try {
                        watchers.get(i).event_CharacterDied(attackedPlayerID, attackedCharacterNr);                      
                    } catch (RemoteException e) {
                    	System.err.println("RemoteException in attackFinished: " + e.getMessage());
                    }
			    }
			}
		} else {
			for (int i = 0; i < watchers.size(); i++) {
                try {
                    watchers.get(i).event_CharacterAttacksNothing(playerID, characterNr);
                } catch (RemoteException e) {
                	System.err.println("RemoteException in attackFinished: " + e.getMessage());
                }
			}
		}
	}

	/**
	 * Pick up an attribute from the current spot.
	 * - Passes functionality on to the Model.
	 * - Notifies the Watchers.
	 * 
	 * @param playerID
	 * @param characterNr
	 * @throws RemoteException
	 */
	public void action_PickUpAttribute(int playerID, int characterNr) throws RemoteException
	{
		Character ch = pf.getPlayerByID(playerID).getCharacter(characterNr);
		int attributeID = pf.pickUp(playerID, characterNr);

		if (attributeID != -1) {
			Attribute attr = ch.getAttributeByID(attributeID);
		    char type = attr.getAttributeChar();
			int room = pf.getRoomIDByCharacter(ch);
			Point pos = getCharacterPosition(ch);

			for (int i = 0; i < watchers.size(); i++) {
				try {
					watchers.get(i).event_AttributeRemoved(room, pos.x, pos.y);
				} catch (RemoteException e) {
					System.err.println("RemoteException in action_PickUpAttribute: " + e.getMessage());
				}
			}

			Watcher w = watchers.getWatcherByPlayerID(playerID);
			if (w != null) {
				try {
					w.event_CharacterPickedUp(playerID, characterNr, attributeID, type);
				} catch (RemoteException e) {
					System.err.println("RemoteException in action_pickUpAttribute: " + e.getMessage());
				}
			}
		}
	}

	/**
	 * Use an attribute.
	 * - Passes functionality on to the Model.
	 * - Notifies the Watchers.
	 * 
	 * @param playerID
	 * @param characterNr
	 * @param attributeID
	 * @throws RemoteException
	 */
	public void action_UseAttribute(int playerID, int characterNr, int attributeID) throws RemoteException
	{
		Attribute a = pf.useAttribute(playerID, characterNr, attributeID);

		if (a != null) {
			Watcher w = watchers.getWatcherByPlayerID(playerID);
			if (w != null) {
		    	if (a.isActive()) {
					try {
						w.event_CharacterUsed(playerID, characterNr, attributeID);
					} catch (RemoteException e) {
						System.err.println("RemoteException in action_UseAttribute: " + e.getMessage());
					}
		    	} else {
					try {
					    w.event_CharacterUsedFinished(playerID, characterNr, attributeID);
					} catch (RemoteException e) {
						System.err.println("RemoteException in action_UseAttribute: " + e.getMessage());
					}
		    	}

				for (int i = 0; i < watchers.size(); i++) {
					try {
					    updateCharacterAttributes(watchers.get(i), playerID, characterNr);
					} catch (RemoteException e) {
						System.err.println("RemoteException in action_UseAttribute 2: " + e.getMessage());
					}
				}
			}
		}
		
	}

	/**
	 * ResetAttribute timer is finished.
	 * - Notifies the Watchers.
	 * 
	 * @param ch
	 * @param attributeID
	 */
	public void useAttributeFinished(Character ch, int attributeID)
	{
		int playerID = pf.getPlayerIDByCharacter(ch);

		if (playerID == -1) {
	        /* Client probably disconnected. */
	        System.err.println("useAttributeFinished: unable to find playerID");
	        return;
	    }

		int characterNr = pf.getPlayerByID(playerID).getCharacterNrByCharacter(ch);

		Watcher w = watchers.getWatcherByPlayerID(playerID);
		if (w != null) {
            try {
                w.event_CharacterUsedFinished(playerID, characterNr, attributeID);
            } catch (RemoteException e) {
            	System.err.println("RemoteException in useAttributeFinished: " + e.getMessage());
            }

            for (int i = 0; i < watchers.size(); i++) {
				try {
				    updateCharacterAttributes(watchers.get(i), playerID, characterNr);
				} catch (RemoteException e) {
					System.err.println("RemoteException in useAttributeFinished: " + e.getMessage());
				}
			}
		}
	}

	/**
	 * Drop an attribute.
	 * - Passes functionality on to the Model.
	 * - Notifies the Watchers.
	 *  
	 * @param playerID
	 * @param characterNr
	 * @param attributeID
	 * @throws RemoteException
	 */
	public void action_DropAttribute(int playerID, int characterNr, int attributeID) throws RemoteException
	{
		Character ch = pf.getPlayerByID(playerID).getCharacter(characterNr);
		Attribute dropped = pf.dropDown(playerID, characterNr, attributeID);

		if (dropped != null) {
		    char type = dropped.getAttributeChar();
			int room = pf.getRoomIDByCharacter(ch);
			Point pos = getCharacterPosition(ch);

			for (int i = 0; i < watchers.size(); i++) {
				try {
					watchers.get(i).event_AttributePlaced(room, type, pos.x, pos.y);
				} catch (RemoteException e ) {
					System.err.println("RemoteException in action_DropAttribute: " + e.getMessage());
				}
			}

			Watcher w = watchers.getWatcherByPlayerID(playerID);
			if (w != null) {
				try {
					w.event_CharacterDropped(playerID, characterNr, attributeID);
				} catch (RemoteException e) {
					System.err.println("RemoteException in action_DropAttribute: " + e.getMessage());
				}
			}
		}
	}

	/**
	 * Place the attributes in the playfield.
	 * 
	 * @param playerID
	 */
    private void placePlayfieldAttributes(int playerID)
    {
        Room rooms[] = pf.getRooms();

        for (int room = 0; room < rooms.length; room++) {
            if (rooms[room] == null)
                continue;

            Spot[][] spots = rooms[room].getSpots();

            for (int y = 0; y < Constants.ROOM_HEIGHT; y++) {
                for (int x = 0; x < Constants.ROOM_WIDTH; x++) {
                    Spot s = spots[x][y];

                    if (s instanceof Floor) {
                        Attribute attr = ((Floor)s).getAttribute();

                        if (attr == null)
                            continue;

                        char type = attr.getAttributeChar();

                        Watcher w = watchers.getWatcherByPlayerID(playerID);
                        if (w == null)
                        	continue;

                        try {
                            w.event_AttributePlaced(room, type, x, y);
                        } catch (RemoteException e) {
                        	System.err.println("RemoteException in placePlayfieldAttributes: " + e.getMessage());
                        }
                    }
				}
			}
        }
    }

    /**
     * Update attributes in the playfield. Called from class Playfield.
     * - Notifies the watchers.
     * 
     * @param floor
     * @param room
     */
    public void updatePlayfieldAttribute(Floor floor, int room)
    {
        Room r = pf.getRoomByID(room);
        Spot[][] spots = r.getSpots();

        for (int y = 0; y < Constants.ROOM_HEIGHT; y++) {
            for (int x = 0; x < Constants.ROOM_WIDTH; x++) {
                if (floor == spots[x][y]) {
                    Attribute attr = floor.getAttribute();
                    char type = attr.getAttributeChar();

                    for (int i = 0; i < watchers.size(); i++) {
                        try {
                            watchers.get(i).event_AttributePlaced(room, type, x, y);
                        } catch (RemoteException e) {
                        	System.err.println("RemoteException in updatePlayfieldAttribute: " + e.getMessage());
                        }
                    }
                }
            }
        }
    }

    /**
     * Update attributes in the view.
     * - Notifies the watcher of Player with Character c.
     * 
     * @param c
     */
    public void updateViewAttributes(Character c)
	{
		int playerID = pf.getPlayerIDByCharacter(c);

		if (playerID == -1) {
	        /* Client probably disconnected. */
	        System.err.println("updateViewAttributes: unable to find playerID");
	        return;
	    }

		int characterNr = pf.getPlayerByID(playerID).getCharacterNrByCharacter(c);

		Watcher watcher = watchers.getWatcherByPlayerID(playerID);
		if (watcher != null) {
            try {
			    updateCharacterAttributes(watcher, playerID, characterNr);
            } catch (RemoteException e) {
            	System.err.println("RemoteException in updateViewAttributes: " + e.getMessage());
            }
		}
	}

    /**
	 * Clone the Character/Player.
	 * - Passes functionality on to the Model.
	 * - Notifies the Watchers.
	 * 
	 * @param playerID
	 * @param direction
     * @throws RemoteException
	 */
	public void action_CloneCharacter(int playerID, int direction) throws RemoteException
	{
		Character c = pf.clone(playerID, direction);

		if (c != null) {
			for (int i = 0; i < watchers.size(); i++) {
				try {
					watchers.get(i).event_NewClone(playerID);
				} catch (RemoteException e) {
					System.err.println("RemoteException in action_CloneCharacter: " + e.getMessage());
				}
			}

			Watcher watcher = watchers.getWatcherByPlayerID(playerID);
			if (watcher != null) {
			    for (int i = 0; i < 2; i++) {
			        try {
					    updateCharacterAttributes(watcher, playerID, i);
			        } catch (RemoteException e) {
			        	System.err.println("RemoteException in action_CloneCharacter: " + e.getMessage());
			        }
			    }
			}

			characterMoved(playerID, 1);
		}
	}

	/**
	 * Send.
	 * 
	 * @param type
	 * @param playerID
	 * @param playerName
	 * @param message
	 */
	public void sendToWatcher(int type, int playerID, String playerName, String message)
	{
		Watcher watcher = watchers.getWatcherByPlayerID(playerID);

		if (watcher != null) {
			try {
				if (type == Constants.ACTION_CHAT)
					watcher.event_SendMessage(Constants.ACTION_CHAT, message, playerName);
				else if (type == Constants.ACTION_TEAMCHAT)
					watcher.event_SendMessage(Constants.ACTION_TEAMCHAT, message, playerName);
				else if (type == Constants.ACTION_YELL)
					watcher.event_SendMessage(Constants.ACTION_YELL, message, playerName);
			} catch (RemoteException e) {
				System.err.println("RemoteException in sendToWatcher: " + e.getMessage());
			}
		}
	}

	/**
	 * Send a message.
	 * 
	 * @param playerID
	 * @param toPlayerID
	 * @param message
	 * @throws RemoteException
	 */
	public void action_Msg(int playerID, int toPlayerID, String message) throws RemoteException
	{
		Watcher watcher = watchers.getWatcherByPlayerID(toPlayerID);

		if (watcher != null) {
			try {
				watcher.event_Msg(message, pf.getPlayerByID(playerID).getName());
			} catch (RemoteException e) {
				System.err.println("RemoteException in action_Msg: " + e.getMessage());
			}
		}
	}

	/**
	 * Redirects the message to playfield.
	 * 
	 * @param type
	 * @param playerID
	 * @param characterNr
	 * @param message
	 */
	public void action_SendMessage(int type, int playerID, int characterNr, String message)
	{
		if (type == Constants.ACTION_TEAMCHAT) {
			pf.teamChat(playerID, message);
		} else if (type == Constants.ACTION_YELL) {
			pf.yellChat(playerID, characterNr, message);
		} else if (type == Constants.ACTION_CHAT) {
			pf.allChat(playerID, message);
		}
	}

	/**
	 * Send a game event.
	 * 
	 * @param message
	 */
	public void gameEvent(String message)
	{
		for (int i = 0; i < watchers.size(); i++) {
            try {
                watchers.get(i).event_GameEvent(message);
            } catch (RemoteException e) {
                System.err.println("RemoteException in gameEvent: " + e.getMessage());
            }
		}
	}

	/**
	 * Updates the countdown on the clients.
	 * 
	 * @param seconds
	 */
	public void countDown(int seconds)
	{
		for (int i = 0; i < watchers.size(); i++) {
            try {
                watchers.get(i).event_CountDown(seconds);
            } catch (RemoteException e) {
                System.err.println("RemoteException in countDown: " + e.getMessage());
            }
		}
	}

	/**
	 * Starts the game.
	 */
	public void startGame()
	{
		int playerID = pf.assignMonster();
		Watcher w = watchers.getWatcherByPlayerID(playerID);

		if (w != null) {
			try {
				w.event_NewMonster(playerID);
			} catch (RemoteException e) {
				System.err.println("RemoteException in startGame: " + e.getMessage());
			}
		}
		characterMoved(playerID, 0);
		
		for (int i = 0; i < watchers.size(); i++) {
			try {
				watchers.get(i).event_StartGame();
			} catch (RemoteException e) {
				System.err.println("RemoteException in startGame: " + e.getMessage());
			}
		}
	}

    /**
     * Ends the game.
     *  
     * @param ch The character who has won the game
     */
    public void endGame(Character ch)
    {
    	if (ch != null) {
	    	int[] win = pf.getWinPlayers(ch);

	    	for (int i = 0; i < win.length; i++) {
	    		Watcher w = watchers.getWatcherByPlayerID(win[i]);
	    		if (w != null) {
		    		try {
						w.event_WinGame();
					} catch (RemoteException e) {
						System.err.println("RemoteException in WinGame: " + e.getMessage());
					}
	    		}
			}

	    	int[] lost = pf.getLostPlayers(ch);

	    	for (int i = 0; i < lost.length; i++) {
	    		Watcher w = watchers.getWatcherByPlayerID(lost[i]);
	    		if (w != null) {
		    		try {
						w.event_LostGame();
					} catch (RemoteException e) {
						System.err.println("RemoteException in LostGame: " + e.getMessage());
					}
	    		}
			}
    	} else {
    		for (int i = 0; i < watchers.size(); i++) {
	    		Watcher w = watchers.get(i);
	    		if (w != null) {
		    		try {
						w.event_EndGame();
					} catch (RemoteException e) {
						System.err.println("RemoteException in EndGame: " + e.getMessage());
					}
	    		}
			}
    	}
    }

	/**
	 * Get spots from a room with a specific ID.
	 * 
	 * @return char[roomID][x][y]
	 */
	private char[][][] getRooms()
	{
		char[][][] rm = new char[pf.getRooms().length][Constants.ROOM_WIDTH][Constants.ROOM_HEIGHT];

		for (int i = 0; i < pf.getRooms().length; i++) {
			Room r = pf.getRoomByID(i);
			Spot[][] spots = r.getSpots();

			for (int y = 0; y < r.getHeight(); y++)
				for (int x = 0; x < r.getWidth(); x++)
					rm[i][x][y] = spots[x][y].getSpotChar();
		}

		return rm;
	}

	/**
	 * Get the X and Y position of Character ch.
	 * 
	 * @param ch
	 * @return Point
	 */
	private Point getCharacterPosition(Character ch)
	{
		int room = pf.getRoomIDByCharacter(ch);
		if (room == -1)
	        return null;

		Room r = pf.getRoomByID(room); 
		Spot[][] spots = r.getSpots();

		for (int y = 0; y < Constants.ROOM_HEIGHT; y++) {
			for (int x = 0; x < Constants.ROOM_WIDTH; x++) {
				Spot s = spots[x][y];
				if (s instanceof CharacterSpot) {
					CharacterSpot cs = (CharacterSpot)s;

					if (cs.getCharacter() == ch)
						return new Point(x, y);
				}
			}
		}

		return null;
	}
}

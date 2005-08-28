/* $Id: Watchable.java,v 1.1 2005-08-28 15:05:08 peter Exp $ */

/**
 * Kiss of Death
 * Watchable.java
 * 
 * Replacement for Observable.
 * This interface needs to be implemented server-side.
 */

package src.watcher;

import java.rmi.*;

public interface Watchable extends Remote
{
    int  action_SignIn(Watcher w, String playerName, char playerType, char gender) throws RemoteException;
    void action_SignOff(int playerID) throws RemoteException;
    void action_MoveCharacter(int playerID, int characterNr, int direction) throws RemoteException;
	void action_RunCharacter(int playerID, int characterNr, int direction) throws RemoteException;
	void action_KissCharacter(int playerID, int characterNr, int direction) throws RemoteException;
	void action_Attack(int playerID, int characterNr, int direction) throws RemoteException;
	void action_Defend(int playerID, int characterNr, int direction) throws RemoteException;
	void action_PickUpAttribute(int playerID, int characterNr) throws RemoteException;
	void action_UseAttribute(int playerID, int characterNr, int attributeID) throws RemoteException;
	void action_DropAttribute(int playerID, int characterNr, int attributeID) throws RemoteException;
	void action_CloneCharacter(int playerID, int direction) throws RemoteException;
	void action_SendMessage(int type, int playerID, int characterNr, String message) throws RemoteException;
	void action_Msg(int playerID, int toplayerID, String message) throws RemoteException;
}

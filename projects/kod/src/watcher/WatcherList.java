/* $Id: WatcherList.java,v 1.1 2005-08-28 15:05:08 peter Exp $ */

/**
 * Kiss of Death
 * WatcherList.java
 * 
 * Creates watcherlist.
 */

package src.watcher;

import java.util.ArrayList;

public class WatcherList
{
	private ArrayList watchers;

	public WatcherList()
	{
		watchers = new ArrayList();
	}

	public synchronized Watcher get(int i)
	{
		return ((WatcherItem)watchers.get(i)).watcher;
	}

    public synchronized int getPlayerID(int i)
	{
		return ((WatcherItem)watchers.get(i)).playerID;
	}

    public synchronized Watcher getWatcherByPlayerID(int playerID)
    {
    	for (int i = 0; i < watchers.size(); i++)
    		if (((WatcherItem)watchers.get(i)).playerID == playerID)
    			return ((WatcherItem)watchers.get(i)).watcher;

    	return null;
    }

    public synchronized void add(Watcher w, int playerID)
    {
    	if (w != null)
    		watchers.add(new WatcherItem(w, playerID));
    }

    public synchronized void remove(Watcher w)
    {
    	for (int i = 0; i < watchers.size(); i++)
    		if (((WatcherItem)watchers.get(i)).watcher == w)
    			watchers.remove(i);
    }

    public synchronized int size()
    {
    	return watchers.size();
    }

    private class WatcherItem
	{
    	public int playerID;
    	public Watcher watcher;

    	public WatcherItem(Watcher watcher, int playerID)
    	{
    		this.watcher = watcher;
    		this.playerID = playerID;
    	}
	}
}

package nl.sogyo.mancala.ui;

import java.util.ArrayList;
import java.util.List;

/**
 * @author Peter Postma
 */
public class EventHandler implements Runnable
{
	private Thread eventHandlerThread;
	private List<EventHandlerObject> eventList;

	public EventHandler()
	{
		this.eventList = new ArrayList<EventHandlerObject>();

		this.eventHandlerThread = new Thread(this);
		this.eventHandlerThread.setDaemon(true);
		this.eventHandlerThread.setName("EventHandler Thread");
		this.eventHandlerThread.start();
	}

	public synchronized void addEvent(EventHandlerObject event)
	{
		this.eventList.add(event);

		notify();
	}

	public void run()
	{
		while (true) {
			synchronized (this) {
				try {
					wait(100);
				} catch (InterruptedException e) {
					// Ignore
				}

				while (this.eventList.size() > 0) {
					EventHandlerObject event = this.eventList.get(0);

					event.handleEvent();

					this.eventList.remove(event);
				}
			}
		}
	}
}

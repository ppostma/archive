package nl.pointless.webmail.web;

/**
 * Listener for events when a panel is switched.
 * 
 * @author Peter Postma
 */
public interface IPanelSwitchListener {

	/**
	 * Event fired when a panel is set invisible.
	 * 
	 * @param panel The panel that became invisible.
	 */
	void onHidePanel(ISwitchablePanel panel);

	/**
	 * Event fired when a panel is set visible.
	 * 
	 * @param panel The panel that became visible.
	 */
	void onShowPanel(ISwitchablePanel panel);
}

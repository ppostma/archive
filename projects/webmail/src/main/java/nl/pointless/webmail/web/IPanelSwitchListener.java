package nl.pointless.webmail.web;

/**
 * Events fired when a panel is switched.
 * 
 * @author Peter Postma
 */
public interface IPanelSwitchListener {

	/**
	 * Event fired when a panel is hidden.
	 * 
	 * @param panel The panel that became invisible.
	 */
	void onHidePanel(ISwitchablePanel panel);

	/**
	 * Event fired when a panel is showed.
	 * 
	 * @param panel The panel that became visible.
	 */
	void onShowPanel(ISwitchablePanel panel);
}

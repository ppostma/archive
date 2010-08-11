package nl.pointless.webmail.web;

import org.apache.wicket.markup.html.panel.Fragment;

/**
 * Interface that a class must implement to be switchable in the content screen.
 * 
 * @author Peter Postma
 */
public interface ISwitchablePanel {

	/**
	 * Make the panel visible.
	 */
	void showPanel();

	/**
	 * Make the panel invisible.
	 */
	void hidePanel();

	/**
	 * Returns the fragment with action buttons.
	 * 
	 * @param id Fragment id.
	 * @return a fragment with action buttons
	 */
	Fragment getActionButtons(String id);
}
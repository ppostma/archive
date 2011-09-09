package nl.pointless.webmail.web.event;

import nl.pointless.webmail.web.ISwitchablePanel;

/**
 * Panel show event.
 * 
 * @author Peter Postma
 */
public class PanelShowEvent {

	private final ISwitchablePanel panel;

	/**
	 * Constructor.
	 * 
	 * @param panel The activated panel.
	 */
	public PanelShowEvent(ISwitchablePanel panel) {
		this.panel = panel;
	}

	public ISwitchablePanel getPanel() {
		return this.panel;
	}
}

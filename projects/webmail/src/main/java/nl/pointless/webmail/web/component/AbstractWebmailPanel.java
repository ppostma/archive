package nl.pointless.webmail.web.component;

import nl.pointless.webmail.web.PanelSwitcher;

import org.apache.wicket.markup.html.panel.Panel;

/**
 * Base class for all webmail panels.
 * 
 * @author Peter Postma
 */
public abstract class AbstractWebmailPanel extends Panel {

	private static final long serialVersionUID = 7377301783230154406L;

	private PanelSwitcher panelSwitcher;

	/**
	 * Constructor.
	 * 
	 * @param id Wicket panel id.
	 * @param aPanelSwitcher A {@link PanelSwitcher}.
	 */
	protected AbstractWebmailPanel(String id, PanelSwitcher aPanelSwitcher) {
		super(id);
		this.panelSwitcher = aPanelSwitcher;
	}

	/**
	 * Activate the message listing panel.
	 */
	protected final void activateMessageListPanel() {
		this.panelSwitcher.setActivePanel("messageListId");
	}

	/**
	 * Activate the message viewing panel.
	 */
	protected final void activateMessageViewPanel() {
		this.panelSwitcher.setActivePanel("messageViewId");
	}

	/**
	 * Activate the message writing panel.
	 */
	protected final void activateMessageWritePanel() {
		this.panelSwitcher.setActivePanel("messageWriteId");
	}

	/**
	 * Activate the previous panel.
	 */
	protected final void activatePreviousPanel() {
		this.panelSwitcher.setActivePanelToPreviousPanel();
	}
}

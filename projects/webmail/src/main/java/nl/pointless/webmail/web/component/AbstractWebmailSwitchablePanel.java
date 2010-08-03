package nl.pointless.webmail.web.component;

import nl.pointless.webmail.web.ISwitchablePanel;
import nl.pointless.webmail.web.PanelSwitcher;

import org.apache.wicket.markup.html.panel.Fragment;

/**
 * Base class for all switchable webmail panels. This class implements
 * {@link ISwitchablePanel} and provides basic functionality needed to make show
 * and hide working.
 * 
 * @author Peter Postma
 */
public abstract class AbstractWebmailSwitchablePanel extends
		AbstractWebmailPanel implements ISwitchablePanel {

	private static final long serialVersionUID = 4211593923111597919L;

	/**
	 * Caching the result of {@link #createActionButtons(String)}.
	 */
	private Fragment actionButtonsFragment;

	/**
	 * Constructor.
	 * 
	 * @param id Wicket panel id.
	 */
	protected AbstractWebmailSwitchablePanel(String id,
			PanelSwitcher panelSwitcher) {
		super(id, panelSwitcher);

		// Initially set invisible.
		this.setVisible(false);
	}

	/**
	 * {@inheritDoc}
	 */
	public final void hidePanel() {
		this.setVisible(false);
	}

	/**
	 * {@inheritDoc}
	 */
	public final void showPanel() {
		this.setVisible(true);
	}

	/**
	 * {@inheritDoc}
	 */
	public final Fragment getActionButtons(String id) {
		if (this.actionButtonsFragment == null) {
			this.actionButtonsFragment = createActionButtons(id);
		}
		return this.actionButtonsFragment;
	}

	/**
	 * Create a fragment with action buttons for this panel.
	 * 
	 * @param id Fragment id.
	 * @return a fragment with action buttons
	 */
	protected abstract Fragment createActionButtons(String id);
}
package nl.pointless.webmail.web;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Deque;
import java.util.LinkedList;
import java.util.List;

import org.apache.wicket.Component;

/**
 * A panel switcher can have a collection of panels but has only one panel
 * active at a time.
 * 
 * @author Peter Postma
 */
public class PanelSwitcher implements Serializable {

	private static final long serialVersionUID = 1L;

	private List<ISwitchablePanel> panels = new ArrayList<ISwitchablePanel>();
	private Deque<ISwitchablePanel> previousPanels = new LinkedList<ISwitchablePanel>();
	private ISwitchablePanel activePanel;

	private IPanelSwitchListener listener;

	/**
	 * Add a panel to the panel switcher.
	 * 
	 * @param panel The Panel to add.
	 */
	public void add(ISwitchablePanel panel) {
		this.panels.add(panel);
	}

	/**
	 * Change the active panel. If the panel was already activated, it will be
	 * reloaded (hide / show).
	 * 
	 * @param panelId Panel Id to make active.
	 */
	public void setActivePanel(String panelId) {
		ISwitchablePanel panel = findPanelById(panelId);
		if (panel == null) {
			throw new IllegalArgumentException("Unable to switch to panel "
					+ panelId + ". Panel not found.");
		}

		if (this.activePanel != null) {
			this.previousPanels.push(this.activePanel);
		}

		setActivePanel(panel);
	}

	/**
	 * Change the active panel to the previous panel, if there was any previous
	 * panel recorded.
	 */
	public void setActivePanelToPreviousPanel() {
		if (this.previousPanels.size() > 0) {
			ISwitchablePanel previousPanel = this.previousPanels.pop();

			setActivePanel(previousPanel);
		}
	}

	private void setActivePanel(ISwitchablePanel panelToActivate) {
		if (this.activePanel != null) {
			this.activePanel.hidePanel();

			if (this.listener != null) {
				this.listener.onHidePanel(this.activePanel);
			}
		}

		this.activePanel = panelToActivate;
		this.activePanel.showPanel();

		if (this.listener != null) {
			this.listener.onShowPanel(this.activePanel);
		}
	}

	private ISwitchablePanel findPanelById(String panelId) {
		ISwitchablePanel foundPanel = null;

		for (ISwitchablePanel panel : this.panels) {
			if (componentIdEquals(panel, panelId)) {
				foundPanel = panel;
				break;
			}
		}

		return foundPanel;
	}

	private boolean componentIdEquals(ISwitchablePanel panel, String panelId) {
		if (!(panel instanceof Component)) {
			throw new IllegalArgumentException(
					"Panel is not a subclass of org.apache.wicket.Component.");
		}

		Component component = (Component) panel;
		return component.getId().equals(panelId);
	}

	public void setPanelSwitchListener(IPanelSwitchListener aListener) {
		this.listener = aListener;
	}
}

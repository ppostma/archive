package nl.pointless.webmail.web.component;

import nl.pointless.webmail.dto.Folder;

import org.apache.wicket.IClusterable;
import org.apache.wicket.extensions.markup.html.repeater.data.table.DataTable;
import org.apache.wicket.extensions.markup.html.repeater.data.table.NavigationToolbar;
import org.apache.wicket.markup.html.WebComponent;
import org.apache.wicket.markup.html.basic.Label;
import org.apache.wicket.markup.html.navigation.paging.PagingNavigator;
import org.apache.wicket.model.IModel;
import org.apache.wicket.model.Model;
import org.apache.wicket.model.StringResourceModel;

/**
 * NavigationToolbar used by {@link MessageListDataTable}.
 * 
 * @author Peter Postma
 */
public class MessageListNavigationToolbar extends NavigationToolbar {

	private static final long serialVersionUID = -2412487068731501961L;

	private MessageListDataTable dataTable;

	/**
	 * Constructor.
	 * 
	 * @param dataTable The message list datatable.
	 */
	public MessageListNavigationToolbar(MessageListDataTable dataTable) {
		super(dataTable);
		this.dataTable = dataTable;
	}

	@Override
	protected PagingNavigator newPagingNavigator(String navigatorId,
			DataTable<?> table) {
		return new MessageListPagingNavigator(navigatorId, table);
	}

	@Override
	protected WebComponent newNavigatorLabel(String navigatorId,
			final DataTable<?> table) {
		return new MessageListNavigatorLabel(navigatorId);
	}

	/**
	 * @return the datatable
	 */
	protected MessageListDataTable getDataTable() {
		return this.dataTable;
	}

	/**
	 * The label for the navigation header.
	 * 
	 * @author Peter Postma
	 */
	private class MessageListNavigatorLabel extends Label {

		private static final long serialVersionUID = 1L;

		public MessageListNavigatorLabel(String id) {
			super(id);
			setDefaultModel(new StringResourceModel("label.navigator", this,
					new Model<LabelModelObject>(new LabelModelObject())));
		}
	}

	/**
	 * Object for to be used for the label model.
	 * 
	 * @author Peter Postma
	 */
	class LabelModelObject implements IClusterable {

		private static final long serialVersionUID = 1L;

		/**
		 * @return "z" in "Showing x to y of z"
		 */
		public int getOf() {
			return getDataTable().getRowCount();
		}

		/**
		 * @return "x" in "Showing x to y of z"
		 */
		public int getFrom() {
			if (getOf() == 0) {
				return 0;
			}
			return (getDataTable().getCurrentPage() * getDataTable()
					.getRowsPerPage()) + 1;
		}

		/**
		 * @return "y" in "Showing x to y of z"
		 */
		public int getTo() {
			if (getOf() == 0) {
				return 0;
			}
			return Math.min(getOf(), getFrom()
					+ getDataTable().getRowsPerPage() - 1);
		}

		/**
		 * @return the name of the open folder
		 */
		public String getFolderName() {
			IModel<Folder> folderModel = getDataTable().getFolderModel();
			return folderModel.getObject().getName();
		}
	}
}
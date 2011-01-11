package nl.pointless.webmail.web.component;

import nl.pointless.webmail.dto.Folder;
import nl.pointless.webmail.dto.Message;

import org.apache.wicket.IClusterable;
import org.apache.wicket.extensions.markup.html.repeater.data.table.DataTable;
import org.apache.wicket.extensions.markup.html.repeater.data.table.NavigationToolbar;
import org.apache.wicket.markup.html.WebComponent;
import org.apache.wicket.markup.html.basic.Label;
import org.apache.wicket.markup.html.navigation.paging.PagingNavigator;
import org.apache.wicket.model.AbstractReadOnlyModel;
import org.apache.wicket.model.IModel;
import org.apache.wicket.model.Model;
import org.apache.wicket.model.StringResourceModel;

/**
 * NavigationToolbar used by {@link MessageListDataTable}.
 * 
 * @author Peter Postma
 */
public class MessageListNavigationToolbar extends NavigationToolbar {

	private static final long serialVersionUID = 1L;

	private IModel<Folder> folderModel;

	/**
	 * Constructor.
	 * 
	 * @param dataTable The message list datatable.
	 * @param folderModel Folder model.
	 */
	public MessageListNavigationToolbar(DataTable<Message> dataTable,
			IModel<Folder> folderModel) {
		super(dataTable);
		this.folderModel = folderModel;
	}

	@Override
	protected PagingNavigator newPagingNavigator(String navigatorId,
			DataTable<?> table) {
		return new MessageListPagingNavigator(navigatorId, table);
	}

	@Override
	protected WebComponent newNavigatorLabel(String navigatorId,
			final DataTable<?> table) {
		return new Label(navigatorId, new AbstractReadOnlyModel<String>() {

			private static final long serialVersionUID = 1L;

			@Override
			public String getObject() {
				MessageListDataProvider dataProvider = (MessageListDataProvider) table
						.getDataProvider();
				String filter = dataProvider.getFilter();

				IModel<LabelModelObject> model = new Model<LabelModelObject>(
						new LabelModelObject());
				IModel<String> theModel;

				if (filter == null) {
					theModel = new StringResourceModel("label.navigator",
							MessageListNavigationToolbar.this, model);
				} else {
					theModel = new StringResourceModel(
							"label.navigator_with_filter",
							MessageListNavigationToolbar.this, model);
				}

				return theModel.getObject();
			}
		});
	}

	@Override
	public boolean isVisible() {
		return getTable().getDataProvider().size() > 0;
	}

	/**
	 * @return the folder model
	 */
	IModel<Folder> getFolderModel() {
		return this.folderModel;
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
			return getTable().getRowCount();
		}

		/**
		 * @return "x" in "Showing x to y of z"
		 */
		public int getFrom() {
			if (getOf() == 0) {
				return 0;
			}
			return (getTable().getCurrentPage() * getTable().getRowsPerPage()) + 1;
		}

		/**
		 * @return "y" in "Showing x to y of z"
		 */
		public int getTo() {
			if (getOf() == 0) {
				return 0;
			}
			return Math.min(getOf(), getFrom() + getTable().getRowsPerPage()
					- 1);
		}

		/**
		 * @return the name of the open folder
		 */
		public String getFolderName() {
			return getFolderModel().getObject().getName();
		}

		/**
		 * @return the search filter
		 */
		public String getSearchFilter() {
			MessageListDataProvider dataProvider = (MessageListDataProvider) getTable()
					.getDataProvider();
			return dataProvider.getFilter();
		}
	}
}
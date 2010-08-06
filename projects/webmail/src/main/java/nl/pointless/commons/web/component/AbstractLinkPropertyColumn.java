package nl.pointless.commons.web.component;

import org.apache.wicket.extensions.markup.html.repeater.data.grid.ICellPopulator;
import org.apache.wicket.extensions.markup.html.repeater.data.table.PropertyColumn;
import org.apache.wicket.markup.html.basic.Label;
import org.apache.wicket.markup.html.link.Link;
import org.apache.wicket.markup.html.panel.Panel;
import org.apache.wicket.markup.repeater.Item;
import org.apache.wicket.model.IModel;

/**
 * A {@link PropertyColumn} containing a link.
 * 
 * @author Peter Postma
 * @param <T> Type of the property.
 */
public abstract class AbstractLinkPropertyColumn<T> extends PropertyColumn<T> {

	private static final long serialVersionUID = 1L;

	/**
	 * Creates a non sortable property column.
	 * 
	 * @param displayModel Display model.
	 * @param propertyExpression Wicket property expression.
	 */
	public AbstractLinkPropertyColumn(IModel<String> displayModel,
			String propertyExpression) {
		super(displayModel, propertyExpression);
	}

	/**
	 * Creates a abstract link property column that is also sortable.
	 * 
	 * @param displayModel Display model.
	 * @param sortProperty Sort property.
	 * @param propertyExpression Wicket property expression.
	 */
	public AbstractLinkPropertyColumn(IModel<String> displayModel,
			String sortProperty, String propertyExpression) {
		super(displayModel, sortProperty, propertyExpression);
	}

	@Override
	public void populateItem(Item<ICellPopulator<T>> item, String componentId,
			IModel<T> model) {
		item.add(new LinkPanel(item, componentId, model));
	}

	protected abstract void onClick(Item<ICellPopulator<T>> item,
			String componentId, IModel<T> model);

	/**
	 * Panel with a link and a label on that link.
	 * 
	 * @author Peter Postma
	 */
	class LinkPanel extends Panel {

		private static final long serialVersionUID = 1L;

		public LinkPanel(final Item<ICellPopulator<T>> item,
				final String componentId, final IModel<T> model) {
			super(componentId);

			Link<Object> link = new Link<Object>("linkId") {

				private static final long serialVersionUID = 1L;

				@Override
				public void onClick() {
					AbstractLinkPropertyColumn.this.onClick(item, componentId,
							model);
				}
			};

			link.add(new Label("labelId", createLabelModel(model))
					.setRenderBodyOnly(true));
			add(link);
		}
	}
}

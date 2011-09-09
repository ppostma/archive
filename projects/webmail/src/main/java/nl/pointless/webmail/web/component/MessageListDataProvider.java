package nl.pointless.webmail.web.component;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.Iterator;
import java.util.List;

import nl.pointless.webmail.dto.Message;

import org.apache.commons.lang.StringUtils;
import org.apache.wicket.extensions.markup.html.repeater.data.sort.SortOrder;
import org.apache.wicket.extensions.markup.html.repeater.util.SortableDataProvider;
import org.apache.wicket.model.IModel;
import org.apache.wicket.model.Model;
import org.apache.wicket.model.PropertyModel;

/**
 * SortableDataProvider for {@link Message} lists.
 * 
 * @author Peter Postma
 */
public class MessageListDataProvider extends SortableDataProvider<Message> {

	private static final long serialVersionUID = 1L;

	private String filter;

	private List<Message> messages;

	private List<Message> unfilteredMessages;

	/**
	 * Constructor.
	 */
	public MessageListDataProvider() {
		this.messages = new ArrayList<Message>();

		setSort("date", SortOrder.DESCENDING);
	}

	/**
	 * {@inheritDoc}
	 */
	public Iterator<Message> iterator(int first, int count) {
		List<Message> newList = new ArrayList<Message>(this.messages);

		final String property = getSort().getProperty();
		final boolean ascending = getSort().isAscending();

		Collections.sort(newList, new Comparator<Message>() {

			public int compare(Message m1, Message m2) {
				PropertyModel<Object> model1 = new PropertyModel<Object>(m1,
						property);
				PropertyModel<Object> model2 = new PropertyModel<Object>(m2,
						property);

				Object modelObject1 = model1.getObject();
				Object modelObject2 = model2.getObject();

				@SuppressWarnings("unchecked")
				int compare = ((Comparable<Object>) modelObject1)
						.compareTo(modelObject2);

				if (!ascending) {
					compare *= -1;
				}

				return compare;
			}
		});

		return newList.subList(first, first + count).iterator();
	}

	/**
	 * {@inheritDoc}
	 */
	public IModel<Message> model(Message message) {
		return new Model<Message>(message);
	}

	/**
	 * {@inheritDoc}
	 */
	public int size() {
		return this.messages.size();
	}

	public void setMessages(List<Message> messages) {
		this.messages = messages;
		this.unfilteredMessages = messages;
		this.filter = null;
	}

	public String getFilter() {
		return this.filter;
	}

	public void setFilter(String filter) {
		if (StringUtils.isNotBlank(filter)) {
			this.filter = filter;
			this.messages = filterMessages();
		} else {
			this.filter = null;
			this.messages = this.unfilteredMessages;
		}
	}

	/**
	 * Filter on subject or sender, ignoring case.
	 * 
	 * @return filtered list of messages
	 */
	private List<Message> filterMessages() {
		List<Message> filteredMessages = new ArrayList<Message>(
				this.unfilteredMessages.size());
		for (Message message : this.unfilteredMessages) {
			if (StringUtils.containsIgnoreCase(message.getSubject(),
					this.filter)
					|| StringUtils.containsIgnoreCase(message.getSender(),
							this.filter)) {
				filteredMessages.add(message);
			}
		}
		return filteredMessages;
	}
}

Feature: Context task dissociation
  As someone collecting tasks
  I can delete a task related to a context
  In order to keep my context meaningful

  Scenario: Removing a task from a context keeps it in the project page it's linked to
    Given I display the "Projects / Calendar1 / Prepare talk about TDD" page
    And there is an item named "Create examples and exercices" in the central list
    And I drop the item on "Contexts / Online" in the page list
    And I display the "Contexts / Online" page
    And there is an item named "Create examples and exercices" in the central list
    When I remove the item
    And I look at the central list
    Then the list does not contain "Create examples and exercices"
    And I display the "Projects / Calendar1 / Prepare talk about TDD" page
    Then there is an item named "Create examples and exercices" in the central list

  Scenario: Removing a task linked only to a context moves it back to the inbox
    Given I display the "Inbox" page
    And I look at the central list
    Then the list is:
     | display          |
    And I display the "Contexts / Errands" page
    And there is an item named "Buy kiwis" in the central list
    When I remove the item
    And I look at the central list
    Then the list does not contain "Buy kiwis"
    And I display the "Inbox" page
    Then there is an item named "Buy kiwis" in the central list

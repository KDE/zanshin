Feature: Editing tasks
  As an organized person
  I can edit a previously created task
  In order to refine its definition and react to change

  Scenario: Editing a task text
    Given I display the "Inbox" page
    And there is an item named "Buy cheese" in the central list
    When I open the item in the editor
    And I change the editor text to "More information"
    And I open the item in the editor again
    Then the editor shows "More information" as text
    And the editor shows "Buy cheese" as title

  Scenario: Editing a task title using the editor
    Given I display the "Inbox" page
    And there is an item named "Buy cheese" in the central list
    When I open the item in the editor
    And I change the editor title to "Borrow cheese"
    And I open the item in the editor again
    Then the editor shows "Borrow cheese" as title
    And there is an item named "Borrow cheese" in the central list

  Scenario: Editing a task title in the central list
    Given I display the "Inbox" page
    And there is an item named "Buy cheese" in the central list
    When I open the item in the editor
    And I rename the item to "Buy better cheese"
    Then the editor shows "Buy better cheese" as title

  Scenario: Editing a task start date
    Given I display the "Inbox" page
    And there is an item named "Buy cheese" in the central list
    When I open the item in the editor
    And I change the editor start date to "2014-06-20"
    And I open the item in the editor again
    Then the editor shows "2014-06-20" as start date

  Scenario: Editing a task due date
    Given I display the "Inbox" page
    And there is an item named "Buy cheese" in the central list
    When I open the item in the editor
    And I change the editor due date to "2014-07-20"
    And I open the item in the editor again
    Then the editor shows "2014-07-20" as due date


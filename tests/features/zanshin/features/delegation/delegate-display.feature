Feature: Displaying delegate on tasks
  As a leader ;-)
  I can see who a task was delegated to
  In order to know who to ask if it's not progressing

  Scenario: Displaying delegate name
    Given I display the "Inbox" page
    And there is an item named "Buy cheese" in the central list
    When I open the item in the editor
    Then the editor shows "John Doe" as delegate


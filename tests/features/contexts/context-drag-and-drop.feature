Feature: Context task association
  As someone collecting tasks
  I can associate tasks to a context
  In order to describe the tasks resources

  @wip
  Scenario: Dropping a task on a context
    Given I display the "Inbox" page
    And there is an item named "Buy cheese" in the central list
    When I drop the item on "Contexts / Errands" in the page list
    And I display the "Contexts / Errands" page
    And I look at the central list
    And I list the items
    Then the list is:
       | display                                       |
       | Buy kiwis                                     |
       | Buy cheese                                    |

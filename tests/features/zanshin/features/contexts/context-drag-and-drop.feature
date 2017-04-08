Feature: Context task association
  As someone collecting tasks
  I can associate tasks to a context
  In order to describe the tasks resources

  Scenario: Dropping a task on a context from the inbox
    Given I display the "Inbox" page
    And there is an item named "Buy rutabagas" in the central list
    When I drop the item on "Contexts / Errands" in the page list
    And I display the "Contexts / Errands" page
    And I look at the central list
    And I list the items
    Then the list is:
       | display                                       |
       | Buy kiwis                                     |
       | Buy rutabagas                                 |

  Scenario: Dropping a task on a context from the project central list
    Given I display the "Projects / Prepare talk about TDD" page
    And there is an item named "Create examples and exercices" in the central list
    When I drop the item on "Contexts / Online" in the page list
    And I display the "Contexts / Online" page
    And I look at the central list
    And I list the items
    Then the list is:
       | display                                                        |
       | Create examples and exercices                                  |
       | Create examples and exercices / Train for the FizzBuzz kata    |
       | Create examples and exercices / Train for the Gilded Rose kata |

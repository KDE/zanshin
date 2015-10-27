Feature: Tag task association
  As someone collecting tasks and notes
  I can associate tasks to a tag
  In order to describe the tasks category

  Scenario: Dropping a task on a tag
    Given I display the "Projects / Party" page
    And there is an item named "Buy a cake" in the central list
    When I drop the item on "Tags / Physics" in the page list
    And I display the "Tags / Physics" page
    And I look at the central list
    And I list the items
    Then the list is:
       | display              |
       | A note about physics |
       | Buy a cake           |


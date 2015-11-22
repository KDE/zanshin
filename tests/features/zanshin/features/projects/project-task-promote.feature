Feature: Task promotion
As someone collecting tasks
I can promote a task into a project
In order to organize my tasks

  Scenario: Task promoted into a project appears in the list
    Given I display the "Projects / Party" page
    And I add a "task" named "Design a present"
    And I look at the central list
    And there is an item named "Design a present" in the central list
    When I promote the item
    And I display the available pages
    And I list the items
    Then the list is:
       | display                           | icon                |
       | Inbox                             | mail-folder-inbox   |
       | Workday                           | go-jump-today       |
       | Projects                          | folder              |
       | Projects / Backlog                | view-pim-tasks      |
       | Projects / Design a present       | view-pim-tasks      |
       | Projects / Party                  | view-pim-tasks      |
       | Projects / Read List              | view-pim-tasks      |
       | Contexts                          | folder              |
       | Contexts / Chores                 | view-pim-tasks      |
       | Contexts / Internet               | view-pim-tasks      |
       | Contexts / Online                 | view-pim-tasks      |


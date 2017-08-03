Feature: Task promotion
As someone collecting tasks
I can promote a task into a project
In order to organize my tasks

  Scenario: Task promoted into a project appears in the list
    Given I display the "Projects / TestData » Calendar1 » Calendar2 / Backlog" page
    And I add a "task" named "Design a present"
    And I look at the central list
    And there is an item named "Design a present" in the central list
    When I promote the item
    And I display the available pages
    And I list the items
    Then the list is:
       | display                                                    | icon                |
       | Inbox                                                      | mail-folder-inbox   |
       | Workday                                                    | go-jump-today       |
       | Projects                                                   | folder              |
       | Projects / TestData » Calendar1                              | folder              |
       | Projects / TestData » Calendar1 / Prepare talk about TDD     | view-pim-tasks      |
       | Projects / TestData » Calendar1 / Read List                  | view-pim-tasks      |
       | Projects / TestData » Calendar1 » Calendar2                    | folder              |
       | Projects / TestData » Calendar1 » Calendar2 / Backlog          | view-pim-tasks      |
       | Projects / TestData » Calendar1 » Calendar2 / Design a present | view-pim-tasks      |
       | Contexts                                                   | folder              |
       | Contexts / Errands                                         | view-pim-notes      |
       | Contexts / Online                                          | view-pim-notes      |


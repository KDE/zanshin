Feature: Context creation
  As someone using tasks
  I can create a context
  In order to give them some semantic


  Scenario: New contexts appear in the list
    Given I display the available pages
    When I add a "context" named "Internet"
    And I list the items
    Then the list is:
       | display                                                | icon                |
       | Inbox                                                  | mail-folder-inbox   |
       | Workday                                                | go-jump-today       |
       | Projects                                               | folder              |
       | Projects / TestData » Calendar1                          | folder              |
       | Projects / TestData » Calendar1 / Prepare talk about TDD | view-pim-tasks      |
       | Projects / TestData » Calendar1 / Read List              | view-pim-tasks      |
       | Projects / TestData » Calendar1 » Calendar2                | folder              |
       | Projects / TestData » Calendar1 » Calendar2 / Backlog      | view-pim-tasks      |
       | Contexts                                               | folder              |
       | Contexts / Errands                                     | view-pim-notes      |
       | Contexts / Internet                                    | view-pim-notes      |
       | Contexts / Online                                      | view-pim-notes      |


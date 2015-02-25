Feature: Project destruction
  As someone collecting tasks and notes
  I can delete a project
  In order to clean my tasks and notes

  Scenario: Removing a simple project from the list
    Given I display the available pages
    When I remove a "project" named "Prepare talk about TDD"
    And I list the items
    Then the list is:
       | display                           | icon                |
       | Inbox                             | mail-folder-inbox   |
       | Projects                          | folder              |
       | Projects / Backlog                | view-pim-tasks      |
       | Projects / Party                  | view-pim-tasks      |
       | Projects / Read List              | view-pim-tasks      |
       | Contexts                          | folder              |
       | Contexts / Chores                 | view-pim-tasks      |
       | Contexts / Internet               | view-pim-tasks      |
       | Contexts / Online                 | view-pim-tasks      |
       | Tags                              | folder              |
       | Tags / Philosophy                 | view-pim-tasks      |
       | Tags / Physics                    | view-pim-tasks      |

Feature: Project creation
  As someone collecting tasks and notes
  I can create a project
  In order to organize my tasks and notes

  Scenario: New projects appear in the list
    Given I display the available pages
    When I add a project named "Birthday" in the source named "TestData/Calendar1"
    And I list the items
    Then the list is:
       | display                           | icon                |
       | Inbox                             | mail-folder-inbox   |
       | Projects                          | folder              |
       | Projects / Backlog                | view-pim-tasks      |
       | Projects / Birthday               | view-pim-tasks      |
       | Projects / Prepare talk about TDD | view-pim-tasks      |
       | Projects / Read List              | view-pim-tasks      |
       | Contexts                          | folder              |
       | Contexts / Chores                 | view-pim-tasks      |
       | Contexts / Internet               | view-pim-tasks      |
       | Contexts / Online                 | view-pim-tasks      |
       | Tags                              | folder              |
       | Tags / Philosophy                 | view-pim-tasks      |
       | Tags / Physics                    | view-pim-tasks      |

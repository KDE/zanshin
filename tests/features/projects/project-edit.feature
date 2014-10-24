Feature: Project rename
  As someone collecting tasks and notes
  I can rename a project
  In order to refine my tasks and notes organization

  Scenario: Renamed projects appear in the list
    Given I display the available pages
    When I rename a "project" named "Birthday" to "Party"
    And I list the items
    Then the list is:
       | display                           | icon              |
       | Contexts                          | folder            |
       | Contexts / Chores                 | view-pim-tasks    |
       | Contexts / Internet               | view-pim-tasks    |
       | Contexts / Online                 | view-pim-tasks    |
       | Inbox                             | mail-folder-inbox |
       | Projects                          | folder            |
       | Projects / Read List              | view-pim-tasks    |
       | Projects / Backlog                | view-pim-tasks    |
       | Projects / Party                  | view-pim-tasks    |
       | Projects / Prepare talk about TDD | view-pim-tasks    |
       | Tags                              | folder            |
       | Tags / Philosophy                 | view-pim-tasks    |
       | Tags / Physics                    | view-pim-tasks    |
